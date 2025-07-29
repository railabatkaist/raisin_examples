// raisin inclusion
#include "raisin_network/node.hpp"
#include "raisin_network/raisin.hpp"

// message inclusion
#include "std_msgs/msg/string.hpp"


using namespace std::chrono_literals;
using namespace raisin;


class EmptySub : public Node{
public:
  EmptySub(std::shared_ptr<Network> network, std::shared_ptr<Remote::Connection> connection) :
          Node(network) {

    // create publisher
    stringSubscriber_ = createSubscriber<std_msgs::msg::String>("string_message", connection,
                                                                std::bind(&EmptySub::messageCallback, this, std::placeholders::_1), "sub");

  };
  ~EmptySub() {
    /// YOU MUST CALL THIS METHOD IN ALL NODES
    cleanupResources();
  }

  void messageCallback(std_msgs::msg::String::SharedPtr message) {
    std::cout<<"message: "<<message->data<<std::endl;
  }
private:
  Subscriber<std_msgs::msg::String>::SharedPtr stringSubscriber_;

};

int main() {
  raisinInit();
  std::vector<std::vector<std::string>> thread_spec = {{std::string("sub")}};
  auto network = std::make_shared<Network>("subscriber", "tutorial", thread_spec);
  std::this_thread::sleep_for(std::chrono::seconds(2));

  auto con = network->connect("publisher");

  EmptySub es(network, con);

  std::this_thread::sleep_for(std::chrono::seconds(20));

  return 0;
}