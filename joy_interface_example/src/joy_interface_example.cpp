//
// Created by user on 25. 7. 29.
//
#include "raisin_network/network.hpp"
#include "raisin_network/node.hpp"
#include "raisin_interfaces/msg/command.hpp"
#include "raisin_interfaces/srv/string.hpp"
#include "raisin_interfaces/srv/string_and_bool.hpp"
#include "raisin_network/raisin.hpp"
#include "std_srvs/srv/trigger.hpp"
#include "raisin_interfaces/msg/velocity_command.hpp"
#include <thread>
using namespace raisin;



int main(int argc, char * argv[])
{
  raisinInit();
  std::string clientId = "joy_interface_example";
  // Instantiate the raisin::Network object as a client
  std::vector<std::vector<std::string>> threads = {{"main"}};
  std::shared_ptr<raisin::Network> network = std::make_shared<raisin::Network>(
          clientId,
          "cmd",
          threads);
  std::this_thread::sleep_for(std::chrono::seconds(3));
  raisin::Node node(network);

  std::string id = "railab_raibo"; // your raibo2_node nickname
  auto connection = network->connect(id);

  std::shared_ptr<raisin::Client<raisin_interfaces::srv::String>> client_ = node.createClient<raisin_interfaces::srv::String>("set_listen", connection, "client");
  auto request_ = std::make_shared<raisin_interfaces::srv::String::Request>();
  request_->data = "vel_cmd/simple";
  auto result_ = client_->asyncSendRequest(request_);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  raisin_interfaces::msg::VelocityCommand cmd;
  auto publisher = node.createPublisher<raisin_interfaces::msg::VelocityCommand>("vel_cmd/simple");

  //you need to add the topic "vel_cmd/simple" to the command_source_topics in the raibo2 config
  while (true) {
    cmd.x_vel = 0;
    cmd.y_vel = 0;
    cmd.yaw_rate = 1;
    publisher->publish(cmd);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return 0;
}
