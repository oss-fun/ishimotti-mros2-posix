#include "mros2.h"
#include "std_msgs/msg/string.hpp"

#include "cmsis_os.h"
#include "netif.h"
#include "netif_posix_add.h"

#include <stdio.h>
#include <string.h>
#include "/home/oss-wasm/Documents/test-mros/vm_share/mros2-posix/workspace/custom_msgs/std_msgs/msg/u_int8_multi_array.hpp"
#include <cartographer_ros_msgs/msg/submap_list.hpp>
#include "cartographer_ros_msgs/msg/submap_query_response.hpp"
#include "/home/oss-wasm/Documents/test-mros/vm_share/mros2-posix/workspace/custom_msgs/service_msgs/msg/SubmapQuery_client.hpp"

void userCallback(cartographer_ros_msgs::msg::SubmapList *msg)
{
  printf("subscribed msg cartographer_ros_msgs::msg::SubmapList\n");
  // printf("subscribed msg: '%s'\r\n", msg->data.c_str());
  // printf("subscribed msg: 'data.size: %d'\r\n", msg->data.size());
  // printf("subscribed msg: '%d'\r\n", msg->msg_size);
  // printf("subscribed msg: 'data[i]: %d'\r\n", msg->data[1]);
  printf("subscribed msg: 'sec: %d'\r\n", msg->sec);
  printf("subscribed msg: 'nanosec: %d'\r\n", msg->nanosec);
  printf("subscribed msg: 'frame_id: %s'\r\n", msg->frame_id.c_str());

  printf("submap size: %d\n", msg->submap.size());
  for (int i = 0; i < msg->submap.size(); i++)
  {
    printf("submap[%d]:\n", i);
    printf("trajectory_id: %d\n", msg->submap[i].trajectory_id);
    printf("submap_index: %d\n", msg->submap[i].submap_index);
    printf("submap_version: %d\n", msg->submap[i].submap_version);
    printf("pose:\n");
    printf("position_x: %lf\n", msg->submap[i].pose.position.x);
    printf("position_y: %lf\n", msg->submap[i].pose.position.y);
    printf("position_z: %lf\n", msg->submap[i].pose.position.z);
    printf("orientation_x: %lf\n", msg->submap[i].pose.orientation.x);
    printf("orientation_y: %lf\n", msg->submap[i].pose.orientation.y);
    printf("orientation_z: %lf\n", msg->submap[i].pose.orientation.z);
    printf("orientation_w: %lf\n", msg->submap[i].pose.orientation.w);
    printf("is_frozen: %d\n", msg->submap[i].is_frozen);
  }

  // // 20秒ストップ
  // osDelay(20000);
  osDelay(3000);
}

std::function<void(cartographer_ros_msgs::msg::SubmapList *)> handleSubmapList;

void handleSubmapListWrapper(cartographer_ros_msgs::msg::SubmapList *msg)
{
  handleSubmapList(msg);
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
  mros2::Publisher pub = node.create_publisher<std_msgs::msg::String>("to_linux", 10);
  // mros2::Subscriber sub = node.create_subscription<std_msgs::msg::UInt8MultiArray>("large_byte_array", 10, userCallback);
  // mros2::Subscriber sub = node.create_subscription<cartographer_ros_msgs::msg::SubmapList>("submap_list", 10, userCallback);

  mros2::Publisher client = node.create_client<service_msgs::msg::SubmapQuery_client>("submap_query", 10);
  // mros2::Publisher pub = node.create_publisher<std_msgs::msg::Int64>("add_two_ints", 10);

  osDelay(100);
  MROS2_INFO("ready to pub/sub message\r\n");

  int publish_period_sec = 100;

  handleSubmapList =
      [&node, &client, publish_period_sec](
          cartographer_ros_msgs::msg::SubmapList *msg)
      -> void
  {
    printf("subscribed msg cartographer_ros_msgs::msg::SubmapList\n");
    printf("subscribed msg: 'sec: %d'\r\n", msg->sec);
    printf("subscribed msg: 'nanosec: %d'\r\n", msg->nanosec);
    printf("subscribed msg: 'frame_id: %s'\r\n", msg->frame_id.c_str());
    printf("submap size: %d\n", msg->submap.size());
    if (msg->submap.size() == 0)
    {
      return;
    }
    printf("service publishing msg: [msg->submap[roop_count]] trajectory_id: '%d' + submap_index: '%d'\r\n", msg->submap[0].trajectory_id, msg->submap[0].submap_index);
    printf("submap version: %d\n", msg->submap[0].submap_version);
    // osDelay(10);
    // absl::MutexLock locker(&mutex_);

    // We do not do any work if nobody listens.
    // if (this->count_publishers(kSubmapListTopic) == 0) {
    //   return;
    // }

    // Keep track of submap IDs that don't appear in the message anymore.
    // std::set<SubmapId> submap_ids_to_delete;
    // for (const auto& pair : submap_slices_) {
    //   submap_ids_to_delete.insert(pair.first);
    // }

    // auto response = client.async_send_request(msg);
    int roop_count = 0;
    for (const auto &submap_msg : msg->submap)
    {

      printf("submap[%d]:\n", roop_count);
      // const SubmapId id{submap_msg.trajectory_id, submap_msg.submap_index};
      // submap_ids_to_delete.erase(id);
      // if ((submap_msg.is_frozen && !FLAGS_include_frozen_submaps) ||
      //     (!submap_msg.is_frozen && !FLAGS_include_unfrozen_submaps))
      // {
      //   continue;
      // }
      // SubmapSlice &submap_slice = submap_slices_[id];
      // submap_slice.pose = ToRigid3d(submap_msg.pose);
      // submap_slice.metadata_version = submap_msg.submap_version;
      // if (submap_slice.surface != nullptr &&
      //     submap_slice.version == submap_msg.submap_version)
      // {
      //   continue;
      // }
      // auto fetched_textures = cartographer_ros::FetchSubmapTextures(
      //     id, client_, callback_group_executor_,
      //     std::chrono::milliseconds(int(publish_period_sec * 1000)));
      // if (fetched_textures == nullptr)
      // {
      //   continue;
      // }
      // CHECK(!fetched_textures->textures.empty());
      // pub.publish(msg);

      // msg初期化
      auto msg_req = service_msgs::msg::SubmapQuery_client();
      msg_req.trajectory_id = submap_msg.trajectory_id;
      msg_req.submap_index = submap_msg.submap_index;
      // msg->submap[i].trajectory_id;
      printf("service publishing msg: [msg->submap[roop_count]] trajectory_id: '%d' + submap_index: '%d'\r\n", msg->submap[roop_count].trajectory_id, msg->submap[roop_count].submap_index);
      printf("service publishing msg: [submap_msg] trajectory_id: '%d' + submap_index: '%d'\r\n", submap_msg.trajectory_id, submap_msg.submap_index);
      printf("service publishing msg: [msg_req] trajectory_id: '%d' + submap_index: '%d'\r\n", msg_req.trajectory_id, msg_req.submap_index);
      printf("service publishing msg: [msg_req] submap_version: '%d'\r\n", submap_msg.submap_version);

      auto response = client.async_send_request(msg_req);
      // // osDelay(1000);

      printf("future.get() start\r\n");
      mros2::spin_until_future_complete(node, &response);
      printf("future.get() end\r\n");
      cartographer_ros_msgs::msg::SubmapQuery_Response msg_test;

      msg_test.copyFromBuf(&response.get()[4]);
      // status.code
      printf("future subscribed msg_test: status:'%d'\r\n", msg_test.status.code);
      printf("future subscribed msg_test: status:'%s'\r\n", msg_test.status.message.c_str());

      // 配列要素分表示

      printf("future subscribed msg_test: cells[%d]:[\r\n", msg_test.cells.size());
      int size = msg_test.cells.size();
      for (int i = 0; i < size; i++)
      {
        printf("%d,", msg_test.cells[i]);
      }
      printf("]\r\n");
      roop_count++;
    }
  };
  // mros2::Subscriber sub = node.create_subscription<cartographer_ros_msgs::msg::SubmapList>("submap_list", 10, userCallback);
  mros2::wait_service(1); // ertps内で照合をmrosgawani
  mros2::Subscriber sub = node.create_subscription<cartographer_ros_msgs::msg::SubmapList>("submap_list", 10, handleSubmapListWrapper);

  // auto count = 0;
  // while (1)
  // {
  //   auto msg = std_msgs::msg::String();
  //   msg.data = "Hello from mros2-posix onto Linux: " + std::to_string(count++);
  //   printf("publishing msg: '%s'\r\n", msg.data.c_str());
  //   pub.publish(msg);
  //   osDelay(1000);
  // }

  mros2::spin();
  return 0;
}
