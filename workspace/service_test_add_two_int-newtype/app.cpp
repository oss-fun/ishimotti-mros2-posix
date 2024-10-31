#include "mros2.h"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/int64.hpp"

#include "cmsis_os.h"
#include "netif.h"
#include "netif_posix_add.h"

#include <stdio.h>
#include <string.h>
// #include "service_msgs/msg/add_two_ints_client.hpp"
#include "/home/oss-wasm/Documents/test-mros/vm_share/mros2-posix/workspace/custom_msgs/service_msgs/msg/add_two_int_client.hpp"

void userCallback(std_msgs::msg::Int64 *msg)
{
  // printf("subscribed msg: '%s'\r\n", msg->data.c_str());
  printf("subscribed msg: calculation sum:'%s'\r\n", msg->data);
}

int main(int argc, char *argv[])
{
  netif_posix_add(NETIF_IPADDR, NETIF_NETMASK);

  osKernelStart();

  printf("mros2-posix start!\r\n");
  printf("app name: echoback_string\r\n");
  mros2::init(0, NULL);
  MROS2_DEBUG("mROS 2 initialization is completed\r\n");

  mros2::Node node = mros2::Node::create_node("mros2_node");
  // mros2::Publisher pub = node.create_publisher<std_msgs::msg::String>("to_linux", 10);
  mros2::Publisher pub = node.create_service_publisher<service_msgs::msg::add_two_int_client>("add_two_intsRequest", 10); //
  // mros2::Publisher pub = node.create_service_publisher<std_msgs::msg::String>("calculator_service", 10); // request_topic_ : CalculatorRequestType servicename:calculator_service
  mros2::Subscriber sub = node.create_service_subscription<std_msgs::msg::Int64>("add_two_intsReply", 10, userCallback); // for add_two_ints
  // mros2::Subscriber sub = node.create_subscription<std_msgs::msg::String>("to_stm", 10, userCallback);

  osDelay(100);
  MROS2_INFO("ready to pub/sub message\r\n");

  auto count = 0;
  while (1)
  {
    auto msg = service_msgs::msg::add_two_int_client();
    msg.a = 3;
    msg.b = 3;

    printf("publishing msg: '%d' + '%d'\r\n", msg.a, msg.b);
    // auto msg = std_msgs::msg::String();
    // msg.data = "Hello from mros2-posix onto Linux: " + std::to_string(count++);
    // printf("publishing msg: '%s'\r\n", msg.data.c_str());
    pub.publish(msg);
    osDelay(1000);
  }

  mros2::spin();
  return 0;
}
