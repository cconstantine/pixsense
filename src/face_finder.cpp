#include <pixsense/face_finder.hpp>

#include <librealsense2/rsutil.h>
#include <opencv2/imgproc/imgproc.hpp>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/string_cast.hpp>

#include <chrono>
#include <thread>

namespace Pixsense {

  float rect_distance(const rs2::depth_frame& depth, const cv::Rect& area) {
    // fprintf(stderr, "rect_distance: depth: (%d, %d)\n", depth.get_width(), depth.get_height());
    // fprintf(stderr, "rect_distance: area:  (%d, %d) x (%d, %d)\n", area.x, area.y, area.width, area.height);
    float sum = 0;
    int count = 0;
    for(int x = 0;x < area.width; x++) {
      for(int y = 0;y < area.height; y++) {
        int x1 = area.x + x;
        int y1 = area.y + y;
        if (x1 < 0 || x1 >= depth.get_width() ||
            y1 < 0 || y1 >= depth.get_height()) {
          continue;
        }
        float d = depth.get_distance(x1, y1);
        if(d > 0) {
          sum += d;
          count++;
        }
      }
    }
    if (count == 0) {
      return 0.0f;
    }

    return sum / count;
  }

  RealsenseTracker::RealsenseTracker() : 
   started(false)
  {
    realsense_context.set_devices_changed_callback([&](rs2::event_information& info)
    {
      update_pipe();
    });   
  }

  bool RealsenseTracker::tick(AbstractFaceTracker& face_detect, glm::vec3 &face_location) {
    try {
      if (started ) {
        // rs2::align align(rs2_stream::RS2_STREAM_COLOR);
        rs2::frameset unaligned_frames = pipes[selected_pipe]->wait_for_frames();

        cv::Rect real_face;
        if(!face_detect.detect(unaligned_frames, real_face)) {
          select_next_pipe();
          return false;
        }

        rs2::depth_frame depths = unaligned_frames.get_depth_frame();
        float            distance = rect_distance(depths, real_face);

        if (distance < 0.100f) {
          select_next_pipe();
          return false;
        }

        rs2_intrinsics intrin = depths.get_profile().as<rs2::video_stream_profile>().get_intrinsics();

        float pixel[2] = {real_face.x + real_face.width / 2.0f, real_face.y + real_face.height / 2.0f };
        rs2_deproject_pixel_to_point(&face_location[0], &intrin, pixel, distance);
        face_location.y = -face_location.y;
        face_location.x = -face_location.x;
        // fprintf(stderr, "%s: %s\n", selected_pipe.c_str(), glm::to_string(face_location).c_str());
        return true;
      } else {
        update_pipe();
      }
    } catch(const std::exception& e) {
      fprintf(stderr, "RealsenseTracker Exception: %s\n", e.what());
      started = false;
      update_pipe();
    }
    return false;
  }

  void RealsenseTracker::select_next_pipe() {
    std::map<std::string, std::shared_ptr<rs2::pipeline>>::const_iterator iter;
    for(iter = pipes.begin(); iter != pipes.end(); iter++) {
      if(iter->first == selected_pipe) {
        iter++;
        break;
      }
    }
    if (iter == pipes.end()) {
      selected_pipe = pipes.begin()->first;
    } else {
      selected_pipe = iter->first;
    }
  }

  void RealsenseTracker::update_pipe() {
    size_t device_count = realsense_context.query_devices().size();
    if(!started && device_count > 0) {
      fprintf(stderr, "RealsenseTracker: starting with %zu devices\n", device_count);

      for (auto&& selected_device : realsense_context.query_devices()) {
        if(started) {
          fprintf(stderr, "Pausing\n");
          std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        std::string device_name = selected_device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
        if (pipes.find(device_name) != pipes.end()) {
          try {
            pipes[device_name]->stop();
          } catch(const std::exception& e) { }
        }
        std::shared_ptr<rs2::pipeline> pipe = std::make_shared<rs2::pipeline>(realsense_context);

        fprintf(stderr, "Device: %s\n", selected_device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
        rs2::config config;
        config.enable_stream(RS2_STREAM_DEPTH, 1280, 720,  RS2_FORMAT_Z16, 30);
        // config.enable_stream(RS2_STREAM_COLOR, 1920, 1080, RS2_FORMAT_RGB8, 30);
        config.enable_stream(RS2_STREAM_INFRARED, 1, 1280, 720, RS2_FORMAT_Y8, 30);

        // config.enable_stream(RS2_STREAM_INFRARED, 2);
        // config.enable_stream(RS2_STREAM_DEPTH, 1);

        pipe->start(config);
        auto depth_sensor = selected_device.first<rs2::depth_sensor>();
        if (depth_sensor.supports(RS2_OPTION_EMITTER_ENABLED)) {
            depth_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 0.f); // Enable emitter
        }
        pipes[device_name] = pipe;
        selected_pipe = device_name;

        started = true;
      }

    } else if (started && device_count == 0) {
      started = false;
      for(auto& iter : pipes) {
        iter.second->stop();
      }
    }
  }

  // Convert rs2::frame to cv::Mat
  cv::Mat AbstractFaceTracker::frame_to_mat(const rs2::frame& f)
  {
      using namespace cv;
      using namespace rs2;

      auto vf = f.as<video_frame>();
      const int w = vf.get_width();
      const int h = vf.get_height();

      if (f.get_profile().format() == RS2_FORMAT_BGR8)
      {
          return Mat(Size(w, h), CV_8UC3, (void*)f.get_data(), Mat::AUTO_STEP);
      }
      else if (f.get_profile().format() == RS2_FORMAT_RGB8)
      {
          auto r = Mat(Size(w, h), CV_8UC3, (void*)f.get_data(), Mat::AUTO_STEP);
          cvtColor(r, r, CV_RGB2BGR);
          return r;
      }
      else if (f.get_profile().format() == RS2_FORMAT_Z16)
      {

          return Mat(Size(w, h), CV_16UC1, (void*)f.get_data(), Mat::AUTO_STEP);
      }
      else if (f.get_profile().format() == RS2_FORMAT_Y8)
      {
          return Mat(Size(w, h), CV_8UC1, (void*)f.get_data(), Mat::AUTO_STEP);
      }

      throw std::runtime_error("Frame format is not supported yet!");
  }

}
