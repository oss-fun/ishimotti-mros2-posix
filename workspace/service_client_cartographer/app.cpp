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
#include "/home/oss-wasm/Documents/test-mros/vm_share/mros2-posix/workspace/custom_msgs/service_msgs/msg/SubmapQuery_client.hpp"
// #include "/home/oss-wasm/Documents/test-mros/vm_share/mros2-posix/workspace/custom_msgs/service_msgs/msg/add_two_ints.hpp"
#include "cartographer_ros_msgs/msg/submap_query_response.hpp"

#include <condition_variable>
// intptr_t *msg_buffer;
// uint8_t *cacheChange_buffer;

// for benchmark
#include <chrono>
#include <fstream>
#include <cstdlib>
#include <memory>
using namespace std::chrono_literals; // for benchmark

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

  mros2::Publisher client = node.create_client<service_msgs::msg::SubmapQuery_client>("submap_query", 10);
  // mros2::Publisher pub = node.create_publisher<std_msgs::msg::Int64>("add_two_ints", 10);
  mros2::wait_service(1); // ertps内で照合をmrosgawani

  osDelay(100);

  MROS2_INFO("ready to pub/sub message\r\n");

  // 計測前に10000回ループ
  for (int i = 0; i < 10; i++)
  {
    auto msg = service_msgs::msg::SubmapQuery_client();
    msg.trajectory_id = 0;
    msg.submap_index = 0;

    printf("publishing msg: trajectory_id: '%d' + submap_index: '%d'\r\n", msg.trajectory_id, msg.submap_index);

    // pub.publish(msg);
    auto response = client.async_send_request(msg);
    // osDelay(1000);

    printf("future.get() start\r\n");
    mros2::spin_until_future_complete(node, &response);
    printf("future.get() return\r\n");

    // std_msgs::msg::Int64 msg_test;
    cartographer_ros_msgs::msg::SubmapQuery_Response msg_test;

    msg_test.copyFromBuf(&response.get()[4]);
    // status.code
    printf("future subscribed msg_test: status:'%d'\r\n", msg_test.status.code);
    // status.string
    printf("future subscribed msg_test: status:'%s'\r\n", msg_test.status.message.c_str());
    // submap_version
    printf("future subscribed msg_test: submap_version:'%d'\r\n", msg_test.submap_version);
    // cells
    printf("future subscribed msg_test: cells:'%d'\r\n", msg_test.cells.size());
    int size = msg_test.cells.size();
    printf("future subscribed msg_test: cells[0]:'%d'\r\n", msg_test.cells[0]);
    printf("future subscribed msg_test: cells[%d]:'%d'\r\n", size - 1, msg_test.cells[size - 1]);

    // 配列要素分表示
    printf("future subscribed msg_test: cells[%d]:[\r\n", size);
    for (int i = 0; i < size; i++)
    {
      printf("%d,", msg_test.cells[i]);
    }
    printf("]\r\n");

    // width
    printf("future subscribed msg_test: width:'%ld'\r\n", msg_test.width);
    // height
    printf("future subscribed msg_test: height:'%ld'\r\n", msg_test.height);

    // resolution
    printf("future subscribed msg_test: resolution:'%f'\r\n", msg_test.resolution);
    // slice_pose
    printf("future subscribed msg_test: slice_pose.position x:'%f',y:'%f',z:'%f'\r\n", msg_test.slice_pose.position.x, msg_test.slice_pose.position.y, msg_test.slice_pose.position.z);
    printf("future subscribed msg_test: slice_pose.orientation x:'%f',y:'%f',z:'%f',w:'%f'\r\n", msg_test.slice_pose.orientation.x, msg_test.slice_pose.orientation.y, msg_test.slice_pose.orientation.z, msg_test.slice_pose.orientation.w);

    // printf("future subscribed msg_test: calculation sum:'%ld'\r\n", msg_test.data);
    osDelay(1000);
    // break;
  }

  // mros2::spin();
  return 0;
}

// std::string client_rq_type = "example_interfaces::srv::dds_::AddTwoInts_Request_";
// std::string client_rr_type = "example_interfaces::srv::dds_::AddTwoInts_Response_";

// mros2::Publisher pub = node.create_client_publisher<service_msgs::msg::add_two_int_client>("add_two_ints", 10, client_rq_type); //
// mros2::Publisher pub = node.create_client_publisher<service_msgs::msg::add_two_ints>("add_two_ints", 10, client_rq_type);        //
// mros2::Subscriber sub = node.create_client_subscription<std_msgs::msg::Int64>("add_two_ints", 10, userCallback, client_rr_type); // for add_two_ints
