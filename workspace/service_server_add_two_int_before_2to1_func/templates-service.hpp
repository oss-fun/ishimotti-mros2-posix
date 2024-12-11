
// #include "std_msgs/msg/string.hpp"
#include "/home/oss-wasm/Documents/test-mros/vm_share/mros2-posix/workspace/custom_msgs/service_msgs/msg/add_two_int_client.hpp"
// #include "/home/oss-wasm/Documents/test-mros/vm_share/mros2-posix/workspace/custom_msgs/service_msgs/msg/add_two_int_request.hpp"

// template mros2::Publisher mros2::Node::create_service_publisher<service_msgs::msg::add_two_int_client>(std::string topic_name, int qos);
// template mros2::Subscriber mros2::Node::create_service_subscription<std_msgs::msg::Int64>(std::string topic_name, int qos, void (*fp)(std_msgs::msg::Int64 *));

template mros2::Publisher mros2::Node::create_service_server_publisher<std_msgs::msg::Int64>(std::string topic_name, int qos); // add_two_ints_server
// template void mros2::Publisher::publish(std_msgs::msg::String &msg);
template mros2::Subscriber mros2::Node::create_service_server_subscription<service_msgs::msg::add_two_int_client, std_msgs::msg::Int64>(std::string topic_name, int qos, void (*fp)(service_msgs::msg::add_two_int_client *, std_msgs::msg::Int64 *)); // add_two_ints_server
template mros2::Subscriber mros2::Node::create_service<service_msgs::msg::add_two_int_client, std_msgs::msg::Int64>(std::string topic_name, int qos, void (*fp)(service_msgs::msg::add_two_int_client *, std_msgs::msg::Int64 *));                     // add_two_ints_server
template void mros2::Subscriber::service_callback_handler<service_msgs::msg::add_two_int_client, std_msgs::msg::Int64>(void *callee, const rtps::ReaderCacheChange &cacheChange);
