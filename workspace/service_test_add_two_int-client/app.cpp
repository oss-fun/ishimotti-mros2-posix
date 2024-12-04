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
#include <condition_variable>
// intptr_t *msg_buffer;
// uint8_t *cacheChange_buffer;

void userCallback(std_msgs::msg::Int64 *msg)
{
  // printf("subscribed msg: '%s'\r\n", msg->data.c_str());
  // printf("subscribed msg: calculation sum:'%ld'\r\n", msg->data);
  // std_msgs::msg::Int64 *msg_test = reinterpret_cast<std_msgs::msg::Int64 *>(&msg_buffer);

  // std_msgs::msg::Int64 msg_test;

  // cacheChange_bufferに入っているポインター表示
  // printf("cacheChange_buffer: %p\r\n", cacheChange_buffer);

  // msg_test.copyFromBuf(&cacheChange_buffer[4]);
  // printf("subscribed msg_test: calculation sum:'%ld'\r\n", msg_test.data);
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

  std::string client_rq_type = "example_interfaces::srv::dds_::AddTwoInts_Request_";
  std::string client_rr_type = "example_interfaces::srv::dds_::AddTwoInts_Response_";

  mros2::Publisher pub = node.create_client_publisher<service_msgs::msg::add_two_int_client>("add_two_ints", 10, client_rq_type);  //
  mros2::Subscriber sub = node.create_client_subscription<std_msgs::msg::Int64>("add_two_ints", 10, userCallback, client_rr_type); // for add_two_ints

  // pub.sub
  //  mros2::Publisher pub = node.create_publisher<std_msgs::msg::String>("to_linux", 10);
  //  mros2::Subscriber sub = node.create_subscription<std_msgs::msg::String>("to_stm", 10, userCallback);

  osDelay(100);
  MROS2_INFO("ready to pub/sub message\r\n");

  while (1)
  {
    auto msg = service_msgs::msg::add_two_int_client();
    msg.a = 3;
    msg.b = 3;

    printf("publishing msg: '%d' + '%d'\r\n", msg.a, msg.b);
    // MROS2_INFO("publishing msg: '%d' + '%d'\r\n", msg.a, msg.b);

    auto response = pub.publish(msg);
    // osDelay(1000);

    printf("future.get() start\r\n");
    mros2::spin_until_future_complete(node, &response);

    printf("future.get() return\r\n");
    std_msgs::msg::Int64 msg_test;
    // uint8_t *result = response.get();
    msg_test.copyFromBuf(&response.get()[4]);
    printf("future subscribed msg_test: calculation sum:'%ld'\r\n", msg_test.data);
  }

  mros2::spin();
  return 0;
}
