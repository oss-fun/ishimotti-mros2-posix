#include "mros2.h"
#include "std_msgs/msg/string.hpp"

#include "cmsis_os.h"
#include "netif.h"
#include "netif_posix_add.h"

#include <stdio.h>
#include <string.h>
#include "/vm_share/mros2-posix/workspace/custom_msgs/std_msgs/msg/u_int8_multi_array.hpp"
#include <cartographer_ros_msgs/msg/submap_list.hpp>
#include "cartographer_ros_msgs/msg/submap_query_response.hpp"
#include "/vm_share/mros2-posix/workspace/custom_msgs/service_msgs/msg/SubmapQuery_client.hpp"
#include "sensor_msgs/msg/occupancy_grid.hpp"
// #include "/vm_share/mros2-posix/mros2/mros2_msgs/sensor_msgs/msg/occupancy_grid.hpp"

#include "cartographer/transform/rigid_transform.h" // cartographer::transform::Rigid3d
#include "absl/memory/memory.h"                     //make_unique
#include "cartographer/io/image.h"                  //absl::make_unique<::cartographer::io::SubmapTextures>
#include "cartographer/io/submap_painter.h"
#include "cartographer/mapping/id.h" //using ::cartographer::mapping::SubmapId;

using ::cartographer::io::PaintSubmapSlicesResult;
using ::cartographer::io::SubmapSlice;
using ::cartographer::mapping::SubmapId;

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

// using Rigid3d = cartographer::transform::Rigid3<double>;
cartographer::transform::Rigid3d ToRigid3d(const geometry_msgs::msg::Pose &pose)
{
  return cartographer::transform::Rigid3d({pose.position.x, pose.position.y, pose.position.z},
                                          Eigen::Quaterniond(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z));
}

/*クラス内変数の定義*/
absl::Mutex mutex_;
std::map<SubmapId, SubmapSlice> submap_slices_ GUARDED_BY(mutex_);
std::string last_frame_id_;
// rclcpp::Time last_timestamp_;
uint32_t last_timestamp_;
const double resolution_ = 0.05; // lanchファイルでの初期値
const double publish_period_sec_ = 1.0;

std::unique_ptr<nav_msgs::msg::OccupancyGrid> CreateOccupancyGridMsg(
    const cartographer::io::PaintSubmapSlicesResult &painted_slices,
    const double resolution, const std::string &frame_id,
    const uint32_t &time)
{
  auto occupancy_grid = absl::make_unique<nav_msgs::msg::OccupancyGrid>();
  const int width = cairo_image_surface_get_width(painted_slices.surface.get());
  const int height =
      cairo_image_surface_get_height(painted_slices.surface.get());

  // cairoの処理結果を表示
  printf("cairo_image_surface_get_width: %d height: %d\n", width, height);

  occupancy_grid->nanosec = time;
  occupancy_grid->frame_id = frame_id;
  occupancy_grid->info.map_load_time = time;
  occupancy_grid->info.resolution = resolution;
  occupancy_grid->info.width = width;
  occupancy_grid->info.height = height;
  occupancy_grid->info.origin.position.x =
      -painted_slices.origin.x() * resolution;
  occupancy_grid->info.origin.position.y =
      (-height + painted_slices.origin.y()) * resolution;
  occupancy_grid->info.origin.position.z = 0.;
  occupancy_grid->info.origin.orientation.w = 1.;
  occupancy_grid->info.origin.orientation.x = 0.;
  occupancy_grid->info.origin.orientation.y = 0.;
  occupancy_grid->info.origin.orientation.z = 0.;

  const uint32_t *pixel_data = reinterpret_cast<uint32_t *>(
      cairo_image_surface_get_data(painted_slices.surface.get()));
  occupancy_grid->data.reserve(width * height);
  for (int y = height - 1; y >= 0; --y)
  {
    for (int x = 0; x < width; ++x)
    {
      const uint32_t packed = pixel_data[y * width + x];
      const unsigned char color = packed >> 16;
      const unsigned char observed = packed >> 8;
      const int value =
          observed == 0
              ? -1
              : ::cartographer::common::RoundToInt((1. - color / 255.) * 100.);
      CHECK_LE(-1, value);
      CHECK_GE(100, value);
      occupancy_grid->data.push_back(value);
    }
  }

  return occupancy_grid;
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
  mros2::Publisher pub = node.create_publisher<nav_msgs::msg::OccupancyGrid>("map", 10);

  // mros2::Publisher client = node.create_client<service_msgs::msg::SubmapQuery_client>("submap_query", 10);
  // mros2::Publisher pub = node.create_publisher<std_msgs::msg::Int64>("add_two_ints", 10);

  osDelay(100);
  MROS2_INFO("ready to pub/sub message\r\n");
  int publish_period_sec = 100;

  // submap_slice_test.poseの表示
  // printf("submap_slice_test.pose: %lf\n", submap_slice_test.pose.translation().x());

  // submap_slice_test.pose = ToRigid3d(submap_msg.pose);
  // submap_slice_test.metadata_version = submap_msg.submap_version;

  // mros2::Subscriber sub = node.create_subscription<cartographer_ros_msgs::msg::SubmapList>("submap_list", 10, userCallback);
  // mros2::wait_service(1); // ertps内で照合をmrosgawani
  // mros2::Subscriber sub = node.create_subscription<cartographer_ros_msgs::msg::SubmapList>("submap_list", 10, handleSubmapListWrapper);

  // osDelay(10000);

  /*データが入ってるかテスト*/
  const SubmapId id_test{0, 0};
  SubmapSlice &submap_slice_test = submap_slices_[id_test];
  // submap_slice_test.resolution
  printf("[Testing-mainfunc]submap_slice_test.resolution: %lf\n", submap_slice_test.resolution);
  // submap_sline_test.width, submap_sline_test.height
  printf("[Testing-mainfunc]submap_slice_test.width: %d height: %d\n", submap_slice_test.width, submap_slice_test.height);

  for (int i = 0; i < 1000; i++)
  {
    /*DrawAndPublish*/
    if (submap_slices_.empty() || last_frame_id_.empty())
    {
      printf("submap_slices_.empty() || last_frame_id_.empty()\n");
      // return;
    }

    auto occupancy_grid = absl::make_unique<nav_msgs::msg::OccupancyGrid>();
    const int width = 10;
    const int height = 10;
    uint32_t time_test = 12345;
    std::string last_frame_id_test = "test_frame_id";

    occupancy_grid->nanosec = time_test;
    occupancy_grid->frame_id = last_frame_id_test;
    occupancy_grid->info.map_load_time = time_test;
    occupancy_grid->info.resolution = resolution_;
    occupancy_grid->info.width = width;
    occupancy_grid->info.height = height;
    occupancy_grid->info.origin.position.x =
        1 * resolution_;
    occupancy_grid->info.origin.position.y =
        (-height + 11) * resolution_;
    occupancy_grid->info.origin.position.z = 0.;
    occupancy_grid->info.origin.orientation.w = 1.;
    occupancy_grid->info.origin.orientation.x = 0.;
    occupancy_grid->info.origin.orientation.y = 0.;
    occupancy_grid->info.origin.orientation.z = 0.;
    // data  を100要素を作って1で埋める
    for (int i = 0; i < 59; i++)
    {
      occupancy_grid->data.push_back(1);
    }
    // occupancy_grid->data.reserve(width * height);

    // auto painted_slices = PaintSubmapSlices(submap_slices_, resolution_);
    // std::unique_ptr<nav_msgs::msg::OccupancyGrid> msg_ptr =
    //     CreateOccupancyGridMsg(painted_slices, resolution_, last_frame_id_,
    //                            last_timestamp_);

    // for width height data
    printf("msg_ptr->data[%d]: [", occupancy_grid->data.size());
    // for (int i = 0; i < msg_ptr->data.size(); i++)
    // {
    //   printf("%d,", msg_ptr->data[i]);
    // }
    // printf("]\n");

    printf("[DrawAndPublish]publishing msg: 'data.size: %d'\r\n", occupancy_grid->data.size());
    pub.publish(*occupancy_grid);
    osDelay(1000);
  }

  // occupancy_grid_publisher_->publish(*msg_ptr);

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
