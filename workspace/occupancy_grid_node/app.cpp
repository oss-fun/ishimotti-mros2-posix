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

/*クラス内変数の定義*/
absl::Mutex mutex_;
std::map<SubmapId, SubmapSlice> submap_slices_ GUARDED_BY(mutex_);
std::string last_frame_id_;
// rclcpp::Time last_timestamp_;
uint32_t last_timestamp_;
const double resolution_ = 0.05; // lanchファイルでの初期値
const double publish_period_sec_ = 1.0;

// using Rigid3d = cartographer::transform::Rigid3<double>;
cartographer::transform::Rigid3d ToRigid3d(const geometry_msgs::msg::Pose &pose)
{
  return cartographer::transform::Rigid3d({pose.position.x, pose.position.y, pose.position.z},
                                          Eigen::Quaterniond(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z));
}

std::unique_ptr<::cartographer::io::SubmapTextures> FetchSubmapTextures(
    const ::cartographer::mapping::SubmapId &submap_id,
    mros2::Publisher client,
    mros2::Node node,
    const std::chrono::milliseconds timeout)
{
  /*FetchSubmapTextures*/
  // msg初期化
  auto msg_req = service_msgs::msg::SubmapQuery_client();
  // auto msg_req = std::make_shared<::service_msgs::msg::SubmapQuery_client>();

  msg_req.trajectory_id = submap_id.trajectory_id;
  msg_req.submap_index = submap_id.submap_index;
  // msg->submap[i].trajectory_id;
  printf("service client msg: [submap_id] trajectory_id: '%d' + submap_index: '%d'\r\n", submap_id.trajectory_id, submap_id.submap_index);

  auto response = client.async_send_request(msg_req);
  // // osDelay(1000);
  printf("future.get() start\r\n");
  mros2::spin_until_future_complete(node, &response); // タイムアウトを設定できるようにする？
  printf("future.get() end\r\n");

  cartographer_ros_msgs::msg::SubmapQuery_Response msg_test;

  msg_test.copyFromBuf(&response.get()[4]);

  // check status
  if (msg_test.status.code != 0 ||
      msg_test.textures.empty())
  {
    return nullptr;
  }

  /*値の確認する*/
  // // status.code
  // printf("future subscribed msg_test: status:'%d'\r\n", msg_test.status.code);
  // // printf("future subscribed msg_test: status:'%s'\r\n", cartographer_ros_msgs::msg::StatusCode::OK);
  // printf("future subscribed msg_test: status:'%s'\r\n", msg_test.status.message.c_str());
  // // 配列要素分表示
  // printf("future subscribed msg_test: cells[%d]:[\r\n", msg_test.textures[0].cells.size());
  // printf("future subscribed msg_test texture[%d]: %d\r\n", msg_test.textures.size());
  // int size = msg_test.textures[0].cells.size();
  // for (int i = 0; i < size; i++)
  // {
  //   printf("%d,", msg_test.textures[0].cells[i]);
  // }
  // printf("]\r\n");

  /*FetchSubmapTextures*/
  auto response_msg = absl::make_unique<::cartographer::io::SubmapTextures>();
  response_msg->version = msg_test.submap_version;
  for (const auto &texture : msg_test.textures)
  {
    const std::string compressed_cells(texture.cells.begin(),
                                       texture.cells.end());
    response_msg->textures.emplace_back(::cartographer::io::SubmapTexture{
        ::cartographer::io::UnpackTextureData(compressed_cells, texture.width,
                                              texture.height),
        texture.width, texture.height, texture.resolution,
        ToRigid3d(texture.slice_pose)});
  }

  return response_msg;
}

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

void DrawAndPublish()
{
  absl::MutexLock locker(&mutex_);
  if (submap_slices_.empty() || last_frame_id_.empty())
  {
    printf("submap_slices_.empty() || last_frame_id_.empty()\n");
    return;
  }

  auto painted_slices = PaintSubmapSlices(submap_slices_, resolution_);
  std::unique_ptr<nav_msgs::msg::OccupancyGrid> msg_ptr =
      CreateOccupancyGridMsg(painted_slices, resolution_, last_frame_id_,
                             last_timestamp_);

  // for width height data
  printf("*** msg_ptr->data[%d]: [", msg_ptr->data.size());
  // for (int i = 0; i < msg_ptr->data.size(); i++)
  // {
  //   printf("%d,", msg_ptr->data[i]);
  // }
  printf("] ***\n");

  printf("[DrawAndPublish]publishing msg: 'data.size: %d'\r\n", msg_ptr->data.size());
  // msg_ptr->info.width,msg_ptr->info.height,msg_prt->info.resolutionの表示
  printf("[DrawAndPublish]publishing msg: 'info.width: %d height: %d resolution: %lf'\r\n", msg_ptr->info.width, msg_ptr->info.height, msg_ptr->info.resolution);
  //  pub.publish(*msg_ptr);

  // 書き出しの日付をつけてdataをファイルに書き出したい
  FILE *fp;
  fp = fopen("/vm_share/occupancy_grid_node_data/mROS2-data.txt", "w");
  // msg_ptr->info.width,msg_ptr->info.height,msg_prt->info.resolutionの書き出し
  fprintf(fp, "%d\n", msg_ptr->info.width);
  fprintf(fp, "%d\n", msg_ptr->info.height);
  fprintf(fp, "%lf\n", msg_ptr->info.resolution);
  fprintf(fp, "%d\n", msg_ptr->info.map_load_time);
  for (int i = 0; i < msg_ptr->data.size(); i++)
  {

    fprintf(fp, "%d,", msg_ptr->data[i]);
  }
  fprintf(fp, "\n");
  fclose(fp);
  printf("\n");
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
  mros2::Publisher client = node.create_client<service_msgs::msg::SubmapQuery_client>("submap_query", 10);
  // mros2::Publisher pub = node.create_publisher<std_msgs::msg::Int64>("add_two_ints", 10);

  osDelay(100);
  MROS2_INFO("ready to pub/sub message\r\n");
  int publish_period_sec = 100;

  handleSubmapList =
      [&node, &client, publish_period_sec, &submap_slices_](
          cartographer_ros_msgs::msg::SubmapList *msg)
      -> void
  {
    // osDelay(10);
    absl::MutexLock locker(&mutex_);

    // We do not do any work if nobody listens.
    // if (this->count_publishers(kSubmapListTopic) == 0) {
    //   return;
    // }

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

    // Keep track of submap IDs that don't appear in the message anymore.
    std::set<SubmapId> submap_ids_to_delete;
    for (const auto &pair : submap_slices_)
    {
      submap_ids_to_delete.insert(pair.first);
    }

    // auto response = client.async_send_request(msg);
    int roop_count = 0;
    for (const auto &submap_msg : msg->submap)
    {
      printf("submap[%d]:\n", roop_count);
      const SubmapId id{submap_msg.trajectory_id, submap_msg.submap_index};
      submap_ids_to_delete.erase(id);
      // if ((submap_msg.is_frozen && !FLAGS_include_frozen_submaps) ||
      //     (!submap_msg.is_frozen && !FLAGS_include_unfrozen_submaps))
      // {
      //   continue;
      // }
      SubmapSlice &submap_slice = submap_slices_[id];
      submap_slice.pose = ToRigid3d(submap_msg.pose);
      submap_slice.metadata_version = submap_msg.submap_version;
      // 既にあるマップはスキップ
      // if (submap_slice.surface != nullptr &&
      //     submap_slice.version == submap_msg.submap_version)
      // {
      //   continue;
      // }

      /*FetchSubmapTextures*/
      auto fetched_textures = FetchSubmapTextures(
          id, client, node,
          std::chrono::milliseconds(int(publish_period_sec * 1000)));

      if (fetched_textures == nullptr)
      {
        printf("----- fetched_textures == nullptr exit handlesunmaplist() -----\n");
        return;
      }
      // CHECK(!fetched_textures->textures.empty());
      // fetched_textures->versionをprint
      printf("fetched_textures->version: %d\n", fetched_textures->version);
      submap_slice.version = fetched_textures->version;

      const auto fetched_texture = fetched_textures->textures.begin();
      submap_slice.width = fetched_texture->width;
      submap_slice.height = fetched_texture->height;
      submap_slice.slice_pose = fetched_texture->slice_pose;
      submap_slice.resolution = fetched_texture->resolution;
      submap_slice.cairo_data.clear();
      submap_slice.surface = ::cartographer::io::DrawTexture(
          fetched_texture->pixels.intensity, fetched_texture->pixels.alpha,
          fetched_texture->width, fetched_texture->height,
          &submap_slice.cairo_data);

      // submap_slice_test.resolution
      printf("[Testing]submap_slice.resolution: %lf\n", submap_slice.resolution);
      // submap_sline_test.width, submap_sline_test.height
      printf("[Testing]submap_slice.width: %d height: %d\n", submap_slice.width, submap_slice.height);

      /*データが入ってるかテスト*/
      const SubmapId id_test{0, 0};
      SubmapSlice &submap_slice_test = submap_slices_[id_test];
      // submap_slice_test.resolution
      printf("[Testing]submap_slice_test.resolution: %lf\n", submap_slice_test.resolution);
      // submap_sline_test.width, submap_sline_test.height
      printf("[Testing]submap_slice_test.width: %d height: %d\n", submap_slice_test.width, submap_slice_test.height);

      roop_count++;
    } // msg->submap分のループ

    // Delete all submaps that didn't appear in the message.
    for (const auto &id : submap_ids_to_delete)
    {
      submap_slices_.erase(id);
    }
    last_timestamp_ = msg->nanosec; // msg->header.stamp;
    last_frame_id_ = msg->frame_id; // msg->header.frame_id;
  };

  // mros2::Subscriber sub = node.create_subscription<cartographer_ros_msgs::msg::SubmapList>("submap_list", 10, userCallback);
  mros2::wait_service(1); // ertps内で照合をmrosgawani
  mros2::Subscriber sub = node.create_subscription<cartographer_ros_msgs::msg::SubmapList>("submap_list", 10, handleSubmapListWrapper);

  // osDelay(10000);

  /*データが入ってるかテスト*/
  const SubmapId id_test{0, 0};
  SubmapSlice &submap_slice_test = submap_slices_[id_test];
  // submap_slice_test.resolution
  printf("[Testing-mainfunc]submap_slice_test.resolution: %lf\n", submap_slice_test.resolution);
  // submap_sline_test.width, submap_sline_test.height
  printf("[Testing-mainfunc]submap_slice_test.width: %d height: %d\n", submap_slice_test.width, submap_slice_test.height);

  for (int i = 0; i < 2000; i++)
  {
    printf("roop_count: %d\n", i);
    DrawAndPublish();
    osDelay(1000);
  }

  mros2::spin();
  return 0;
}
