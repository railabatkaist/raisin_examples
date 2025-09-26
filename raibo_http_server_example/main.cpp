#include <crow.h>  // or: #include <crow/crow.h> depending on your vcpkg port

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <random>
#include <set>

#include "raisin_interfaces/msg/robot_state.hpp"
#include "raisin_network/raisin.hpp"
#include "raisin_interfaces/msg/ffmpeg_packet.hpp"
#include "ffmpeg_image_transport/ffmpeg_decoder.hpp"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs.hpp" // For encoding the image to JPEG
#include "opencv2/imgproc.hpp" // For drawing text

static const char kIndexHtml[] = R"(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>Robot Status</title>
</head>
<body>
  <h1>Hello, Server is Running!</h1>
  <p>Robot state will appear here.</p>
</body>
</html>
)";

class RobotCommunicator : public raisin::Node {
public:
  RobotCommunicator(std::shared_ptr<raisin::Network> network,
                    const std::shared_ptr<raisin::Remote::Connection>& connection,
                    std::set<crow::websocket::connection*>& video_users)
    : Node(network),
      video_users_(video_users),
      random_engine_(std::random_device{}()),
      // Initialize the distribution to the range [0.0, 1.0]
      noise_dist_(0.0, 1.0) {
    // Subscribe to your RobotState topic
    stateSubscriber_ =
        createSubscriber<raisin::raisin_interfaces::msg::RobotState>(
            "string_message", connection,
            [this]<typename T0>(T0&& PH1) {
              stateCallback(std::forward<T0>(PH1));
            },
            "sub");

    videoSubscriber_ =
        createSubscriber<raisin::raisin_interfaces::msg::FfmpegPacket>(
            "video", connection,
            [this](auto&& PH1) {
              compressedImageCallback(std::forward<decltype(PH1)>(PH1));
            },
            "sub");

    codecMap_["libx264"] = "h264";
    codecMap_["libaom-av1"] = "libaom-av1";

    state_.body_temperature = 40.0;
    state_.voltage = 12.0;
    state_.actuator_states.resize(12);

    for (auto& a : state_.actuator_states) {
      a.effort = 0.1;
      a.temperature = 40.0;
    }
  }

  ~RobotCommunicator() override { cleanupResources(); }

  void stateCallback(
      const raisin::raisin_interfaces::msg::RobotState::SharedPtr& state) {
    std::scoped_lock lock(stateMutex_);
    state_ = *state;
  }

  [[nodiscard]] raisin::raisin_interfaces::msg::RobotState getState() {
    std::scoped_lock lock(stateMutex_);

    /// only for debugging //////////////////
    raisin::raisin_interfaces::msg::RobotState noisy_state = state_;

    // Add random noise to the copied state
    noisy_state.body_temperature += noise_dist_(random_engine_);
    noisy_state.voltage += noise_dist_(random_engine_);

    for (auto& a : noisy_state.actuator_states) {
      // Adding noise between -0.5 and 0.5 to make it fluctuate around the initial value
      double centered_noise = noise_dist_(random_engine_) - 0.5;
      a.effort += centered_noise;
      a.temperature += centered_noise * 10.0;
      // Scale noise for more visible temp change
    }
    //////////////////////////////////////////

    return noisy_state;

    // return state_;
  }

  void compressedImageCallback(
      const raisin::raisin_interfaces::msg::FfmpegPacket::ConstSharedPtr msg) {
    if (!decoder_.isInitialized()) {
      decoder_.initialize(
          msg, std::bind(&RobotCommunicator::imageCallback, this,
                         std::placeholders::_1),
          codecMap_[msg->encoding]);
    }

    decoder_.decodePacket(msg);
  }

  void imageCallback(const ffmpeg_image_transport::ImageConstPtr& img) {
    cv::Mat bgrImage(int(img->height), int(img->width), CV_8UC3, (void*)img->data.data());
    // Change: Encode and broadcast the frame
    std::vector<uchar> buffer;
    // Encode to JPEG format with 80% quality
    cv::imencode(".jpg", bgrImage, buffer, {cv::IMWRITE_JPEG_QUALITY, 80});

    // Create a string_view for efficient sending (avoids copying)
    std::string_view data(reinterpret_cast<const char*>(buffer.data()), buffer.size());

    // Broadcast the JPEG data to all connected WebSocket clients
    for (auto user : video_users_) {
      user->send_binary(std::string(data));
    }
  }

private:
  raisin::Subscriber<raisin::raisin_interfaces::msg::RobotState>::SharedPtr
    stateSubscriber_;
  raisin::Subscriber<raisin::raisin_interfaces::msg::FfmpegPacket>::SharedPtr
    videoSubscriber_;

  raisin::raisin_interfaces::msg::RobotState state_;

  // ffmpeg
  ffmpeg_image_transport::FFMPEGDecoder decoder_;
  std::map<std::string, std::string> codecMap_;

  std::set<crow::websocket::connection*>& video_users_;

  // only for debugging
  std::mutex stateMutex_;
  std::mt19937 random_engine_;
  std::uniform_real_distribution<double> noise_dist_;
};

/// for debugging only
static void runVideoStreamDebugger(std::set<crow::websocket::connection*>& users,
                                   std::mutex& mutex) {
  int frame_counter = 0;
  while (true) {
    // 1. Wait for 1 second
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 2. Create a black image
    cv::Mat frame = cv::Mat::zeros(480, 640, CV_8UC3);

    // 3. Prepare text to draw
    std::string text = "Frame: " + std::to_string(frame_counter++);
    int font_face = cv::FONT_HERSHEY_SIMPLEX;
    double font_scale = 2.0;
    int thickness = 3;
    cv::Point text_origin(50, 240); // Position of the text

    // 4. Draw the number on the image
    cv::putText(frame, text, text_origin, font_face, font_scale,
                cv::Scalar(255, 255, 255), thickness, cv::LINE_AA);

    // 5. Encode the image to JPEG format
    std::vector<uchar> buffer;
    cv::imencode(".jpg", frame, buffer, {cv::IMWRITE_JPEG_QUALITY, 90});
    std::string_view data(reinterpret_cast<const char*>(buffer.data()),
                          buffer.size());

    // 6. Broadcast the frame to all WebSocket clients (thread-safe)
    {
      std::lock_guard<std::mutex> _(mutex);
      for (auto user : users) {
        user->send_binary(std::string(data));
      }
    }
  }
}

// ---- Convert your RobotState to Crow JSON -----------------------------------
static crow::json::wvalue toJson(
    const raisin::raisin_interfaces::msg::RobotState& s) {
  crow::json::wvalue j;

  // scalars you printed
  j["body_temperature"] = s.body_temperature;
  j["voltage"] = s.voltage;

  // first 12 actuator states (same loop bounds as your cout)
  crow::json::wvalue::list actuators;
  actuators.reserve(12);
  const auto n = std::min<std::size_t>(12, s.actuator_states.size());
  for (std::size_t i = 0; i < n; ++i) {
    const auto& a = s.actuator_states[i];
    crow::json::wvalue item;
    item["effort"] = a.effort;
    item["temperature"] = a.temperature;
    actuators.push_back(std::move(item));
  }
  j["actuator_states"] = std::move(actuators);

  return j;
}

// -----------------------------------------------------------------------------

int main(int /*argc*/, char** /*argv*/) {
  // --- RAISIN network setup
  raisin::raisinInit();

  std::vector<std::vector<std::string>> thread_spec = {{std::string("sub")}};
  auto network = std::make_shared<raisin::Network>("http_server", "http_server",
                                                   thread_spec);

  // Give the network a moment if your transport needs it
  std::this_thread::sleep_for(std::chrono::seconds(2));

  auto connection = network->connect("raibo");
  std::set<crow::websocket::connection*> video_users;
  std::mutex users_mutex; // Mutex to protect access to the set

  RobotCommunicator raisinClient(network, connection, video_users);

  // --- Crow HTTP server
  crow::SimpleApp app;

  CROW_ROUTE(app, "/")([] {
    crow::response r{kIndexHtml};
    r.add_header("Content-Type", "text/html; charset=utf-8");
    return r;
  });

  CROW_ROUTE(app, "/health")([] { return crow::response{200, "ok"}; });

  CROW_ROUTE(app, "/state").methods(crow::HTTPMethod::GET,
                                    crow::HTTPMethod::OPTIONS)
      ([&raisinClient](const crow::request& req) {
        if (req.method == crow::HTTPMethod::OPTIONS) {
          crow::response pre{204};
          pre.add_header("Access-Control-Allow-Origin", "*");
          pre.add_header("Access-Control-Allow-Methods", "GET, OPTIONS");
          pre.add_header("Access-Control-Allow-Headers", "Content-Type");
          return pre;
        }
        auto state = raisinClient.getState();
        crow::response r{toJson(state)};
        r.add_header("Access-Control-Allow-Origin", "*");
        // or your site’s origin
        r.add_header("Access-Control-Allow-Methods", "GET, OPTIONS");
        r.add_header("Access-Control-Allow-Headers", "Content-Type");
        return r;
      });

  CROW_WEBSOCKET_ROUTE(app, "/video")
      .onopen([&](crow::websocket::connection& conn) {
          std::lock_guard<std::mutex> _(users_mutex);
          video_users.insert(&conn);
          CROW_LOG_INFO << "New video client connected: " << &conn;
      })
      .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t code) { // All three arguments
          std::lock_guard<std::mutex> _(users_mutex);
          video_users.erase(&conn);
          CROW_LOG_INFO << "Video client disconnected: " << &conn << " Reason: " << reason << " Code: " << code;
      });

  // Launch the video stream debugger in a separate thread
  std::thread video_debugger_thread(runVideoStreamDebugger, std::ref(video_users),
                                    std::ref(users_mutex));
  video_debugger_thread.detach(); // Allow the thread to run in the background

  // Configure port via env PORT or default to 18080
  unsigned short port = 18080;
  if (const char* p = std::getenv("PORT")) {
    try {
      port = static_cast<unsigned short>(std::stoi(p));
    } catch (...) {
    }
  }

  app.port(port).multithreaded().run();
  return 0;
}