//
// Created by munjuhyeok on 25. 7. 29.
// raisin inclusion
#include "raisin_network/node.hpp"
#include "raisin_network/raisin.hpp"

// message inclusion
#include "raisin_interfaces/srv/string.hpp"

// std inclusion
#include <string>

using namespace raisin;


class EmptySerivce : public Node{
  public:
    EmptySerivce(std::shared_ptr<Network> network) :
    Node(network) {

      // create Service
      stringService_ = createService<raisin_interfaces::srv::String>("string_service",
                          std::bind(&EmptySerivce::responseCallback, this, std::placeholders::_1, std::placeholders::_2), "service");

    };
    ~EmptySerivce() {
      /// YOU MUST CALL THIS METHOD IN ALL NODES
      cleanupResources();
    }

    void responseCallback(raisin_interfaces::srv::String::Request::SharedPtr request,
                                  raisin_interfaces::srv::String::Response::SharedPtr response) {
      response->success = true;
      response->message = request->data + ": response";
    }
  private:
    Service<raisin_interfaces::srv::String>::SharedPtr stringService_;
};

int main() {
  raisinInit();
  std::vector<std::vector<std::string>> thread_spec = {{std::string("service")}};
  auto network = std::make_shared<Network>("service", "tutorial", thread_spec);
  network->launchServer(Remote::NetworkType::TCP);

  EmptySerivce es(network);

  std::this_thread::sleep_for(std::chrono::seconds(20));
  return 0;
}