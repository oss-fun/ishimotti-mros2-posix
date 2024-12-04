#include "mros2.h"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/int64.hpp"

#include "cmsis_os.h"
#include "netif.h"
#include "netif_posix_add.h"

#include <stdio.h>
#include <string.h>
// #include "service_msgs/msg/add_two_ints_client.hpp"
// #include "/home/oss-wasm/Documents/test-mros/vm_share/mros2-posix/workspace/custom_msgs/service_msgs/msg/add_two_int_client.hpp"
#include "/home/oss-wasm/Documents/test-mros/vm_share/mros2-posix/workspace/custom_msgs/service_msgs/msg/add_two_int_request.hpp"

// スレッドセーフなキューまたはマップを使用
// std::mutex bufferMutex;
// std::queue<CacheChangeInfo> cacheChangeQueue;

mros2::Publisher pub_callback;
void userCallback(service_msgs::msg::add_two_int_request *msg)
{
  // printf("subscribed msg: '%s'\r\n", msg->data.c_str());
  // printf("subscribed msg: calculation sum:'%s'\r\n", msg->data);
  printf("subscribed msg: calculation sum:'%d' + '%d'\r\n", msg->a, msg->b);
  auto response = std_msgs::msg::Int64();
  response.data = msg->a + msg->b;
  printf("publishing msg: calculation sum:'%d'\r\n", response.data);
  printf("NO-PUB publishing msg: calculation sum:'%d'\r\n", response.data);
  pub_callback.publish(response);
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

  // add_two_intsRequestで動く形で、Subscribe実装　topic名　add_two_intsRequest
  mros2::Subscriber sub = node.create_service_server_subscription<service_msgs::msg::add_two_int_request>("add_two_intsRequest", 10, userCallback);

  // add_two_intsReplyで動く形で、Publish実装　topic名　add_two_intsReply
  mros2::Publisher pub = node.create_service_server_publisher<std_msgs::msg::Int64>("add_two_intsReply", 10);
  pub_callback = pub;

  osDelay(100);
  MROS2_INFO("ready to pub/sub message\r\n");

  auto count = 0;
  while (1)
  {
    // auto msg = service_msgs::msg::Int64();
    // msg.data = count++;

    // printf("publishing msg: '%d' + '%d'\r\n", msg.a, msg.b);
    // auto msg = std_msgs::msg::String();
    // msg.data = "Hello from mros2-posix onto Linux: " + std::to_string(count++);
    // printf("publishing msg: '%s'\r\n", msg.data.c_str());
    // pub.publish(msg);
    osDelay(1000);
  }

  mros2::spin();
  return 0;
}
