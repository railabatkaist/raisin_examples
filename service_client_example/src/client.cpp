// raisin inclusion
#include "raisin_network/node.hpp"
#include "raisin_network/raisin.hpp"

// message inclusion
#include "raisin_interfaces/srv/string.hpp"

using namespace std::chrono_literals;
using namespace raisin;

class EmptyClient : public Node{
public:
  EmptyClient(std::shared_ptr<Network> network, std::shared_ptr<Remote::Connection> connection) :
          Node(network) {

    // create Service
    stringClient_ = createClient<raisin_interfaces::srv::String>("string_service", connection, "client");
    createTimedLoop("request_repeat", [this](){
                      if (stringClient_ && stringClient_->isServiceAvailable()) {
                        if (!future_.valid()) {
                          auto req = std::make_shared<raisin_interfaces::srv::String::Request>();
                          req->data = "request";
                          future_ = stringClient_->asyncSendRequest(req);
                          std::cout << "sent request " << std::endl;
                        }

                        if (future_.valid() && future_.wait_for(0s) == std::future_status::ready) {
                          auto response = future_.get();
                          future_ = {};
                          std::cout << "message " << response->message << std::endl;
                        }
                      }
                    }
            , 1.);
  };
  ~EmptyClient() {
    /// YOU MUST CALL THIS METHOD IN ALL NODES
    cleanupResources();
  }

  void responseCallback(raisin_interfaces::srv::String::Response::SharedPtr response) {
    std::cout<<"response: "<<response->message<<std::endl;
  }
private:
  Client<raisin_interfaces::srv::String>::SharedPtr stringClient_;
  Client<raisin_interfaces::srv::String>::SharedFuture future_;

};


int main() {
  raisinInit();
  std::vector<std::vector<std::string>> thread_spec = {{std::string("client")}};
  auto network = std::make_shared<Network>("client", "tutorial", thread_spec);
  std::this_thread::sleep_for(std::chrono::seconds(2));

  auto con = network->connect("service");

  EmptyClient ec(network, con);

  std::this_thread::sleep_for(std::chrono::seconds(20));

  return 0;
}