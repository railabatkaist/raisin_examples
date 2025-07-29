//
// Created by munjuhyeok on 25. 7. 29.
//
// raisin inclusion
#include "raisin_network/node.hpp"
#include "raisin_network/raisin.hpp"

// message inclusion
#include "std_msgs/msg/string.hpp"

// std inclusion
#include <string>

using namespace raisin;


class EmptyPub : public Node{
public:
  EmptyPub(std::shared_ptr<Network> network) :
          Node(network){

    // create publisher
    stringPublisher_ = createPublisher<std_msgs::msg::String>("string_message");
    createTimedLoop("string_message", [this](){
      std_msgs::msg::String msg;
      msg.data = "raisin publisher!";
      stringPublisher_->publish(msg);
    }, 1., "pub");
  };
  ~EmptyPub() {
    /// YOU MUST CALL THIS METHOD IN ALL NODES
    cleanupResources();
  }
private:
  Publisher<std_msgs::msg::String>::SharedPtr stringPublisher_;
};


int main() {
  raisinInit();
  std::vector<std::vector<std::string>> thread_spec = {{std::string("pub")}};
  auto network = std::make_shared<Network>("publisher", "tutorial", thread_spec);
  network->launchServer(Remote::NetworkType::TCP);

  EmptyPub ps(network);

  std::this_thread::sleep_for(std::chrono::seconds(20));
  return 0;
}