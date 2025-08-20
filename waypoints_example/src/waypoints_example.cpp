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
#include "raisin_interfaces/srv/set_waypoints.hpp"
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
  raisin::Node node(network);

  std::string id = "railab_raibo"; // your raibo2_node nickname
  auto connection = network->connect(id);
  {
    std::shared_ptr<raisin::Client<raisin_interfaces::srv::String>> client_ = node.createClient<raisin_interfaces::srv::String>("set_listen", connection, "client");
    auto request_ = std::make_shared<raisin_interfaces::srv::String::Request>();
    request_->data = "vel_cmd/autonomy";
    auto result_ = client_->asyncSendRequest(request_);
  }
  {
    std::shared_ptr<raisin::Client<raisin_interfaces::srv::SetWaypoints>> client_ = node.createClient<raisin_interfaces::srv::SetWaypoints>("planning/set_waypoints", connection, "client");
    auto request_ = std::make_shared<raisin_interfaces::srv::SetWaypoints::Request>();
    raisin_interfaces::msg::Waypoint wp;
    wp.frame = connection->id;
    wp.x = 3;
    wp.y = 0;
    wp.z = 0;
    wp.use_z = false;
    request_->waypoints.push_back(wp);
    request_->repetition = 1; // TODO: add repetition
    request_->current_index = 0; // TODO: add current index
    auto result_ = client_->asyncSendRequest(request_);
    std::cout<<wp.frame<<std::endl;
  }
  node.cleanupResources();

  return 0;
}
