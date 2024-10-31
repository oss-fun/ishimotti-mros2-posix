#include "mros2.h"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/int64.hpp"

#include "cmsis_os.h"
#include "netif.h"
#include "netif_posix_add.h"

#include <stdio.h>
#include <string.h>

void userCallback(std_msgs::msg::Int64 *msg)
{
  // printf("subscribed msg: '%s'\r\n", msg->data.c_str());
  printf("subscribed msg: reply_num %ld \r\n", msg->data);
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
  mros2::Publisher pub = node.create_service_publisher<std_msgs::msg::String>("add_two_intsRequest", 10); //

  // mros2::Publisher pub = node.create_service_publisher<std_msgs::msg::String>("calculator_service", 10); // request_topic_ : CalculatorRequestType servicename:calculator_service
  mros2::Subscriber sub = node.create_service_subscription_debug<std_msgs::msg::Int64>("add_two_intsReply", 10, userCallback); // for add_two_ints topic_nameにrtがついている
  // mros2::Subscriber sub = node.create_subscription<std_msgs::msg::Int64>("add_two_intsReply", 10, userCallback); // for add_two_ints
  // mros2::Subscriber sub = node.create_subscription<std_msgs::msg::String>("to_stm", 10, userCallback);

  osDelay(100);
  MROS2_INFO("ready to pub/sub message\r\n");

  auto count = 0;
  while (1)
  {
    int i;
    i++;
    auto msg = std_msgs::msg::String();
    // msg.data = "Hello from mros2-posix onto Linux: " + std::to_string(count++);//44 現状Max23byte
    // msg.data = "mros2-posix" + std::to_string(count++);//ここをint64二つの値に変更したい
    // printf("publishing msg: '%s'\r\n", msg.data.c_str());

    // int64_t a = 3;
    // int64_t b = 4;
    // char buffer[16];

    // int64_t 型の値をリトルエンディアンでバッファにコピー
    // for (int i = 0; i < 8; ++i)
    // {
    //   buffer[i] = (char)((a >> (8 * i)) & 0xFF);
    //   buffer[8 + i] = (char)((b >> (8 * i)) & 0xFF);
    // }
    // バッファを文字列として msg.data に設定
    // msg.data = std::string(buffer, 16);

    char buffer[16] = {0};
    // リトルエンディアン形式で値を設定
    buffer[0] = 0x03; // 3の下位バイト
    // buffer[1] から buffer[7] は既に0
    buffer[8] = 0x04; // 4の下位バイト
    // msg.data = buffer;
    // msg.data = std::string(buffer, 14);
    // std::string("\x03\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00", 16);
    msg.data = std::string("\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00", 12); // a、bのうち片方は12固定で、もう一方だけ値を変更できる

    // printf("送信する値: a = %lld, b = %lld\r\n", a, b);
    printf("publishing msg: '%s'[%d]\r\n", msg.data.c_str(), i);
    pub.publish(msg);
    osDelay(1000);
  }

  mros2::spin();
  return 0;
}
