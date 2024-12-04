
// #include "std_msgs/msg/string.hpp"
// #include "/home/oss-wasm/Documents/test-mros/vm_share/mros2-posix/workspace/custom_msgs/service_msgs/msg/add_two_int_client.hpp"

// template mros2::Publisher mros2::Node::create_publisher<std_msgs::msg::String>(std::string topic_name, int qos);
template mros2::Publisher mros2::Node::create_client_publisher<service_msgs::msg::add_two_int_client>(std::string topic_name, int qos, std::string type_name);
// template void mros2::Publisher::publish(std_msgs::msg::String &msg);
template mros2::Subscriber mros2::Node::create_client_subscription<std_msgs::msg::Int64>(std::string topic_name, int qos, void (*fp)(std_msgs::msg::Int64 *), std::string type_name);
// template mros2::Subscriber mros2::Node::create_subscription(std::string topic_name, int qos, void (*fp)(std_msgs::msg::String *));
// template void mros2::Subscriber::callback_handler<std_msgs::msg::String>(void *callee, const rtps::ReaderCacheChange &cacheChange);
