
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/u_int8_multi_array.hpp"


template mros2::Publisher mros2::Node::create_publisher<std_msgs::msg::String>(std::string topic_name, int qos);
template void mros2::Publisher::publish(std_msgs::msg::String &msg);



template mros2::Subscriber mros2::Node::create_subscription(std::string topic_name, int qos, void (*fp)(std_msgs::msg::UInt8MultiArray*));
template void mros2::Subscriber::callback_handler<std_msgs::msg::UInt8MultiArray>(void *callee, const rtps::ReaderCacheChange &cacheChange);
