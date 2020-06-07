#include <pixsense/face_finder.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <limits>

#include <librealsense2/rsutil.h>

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

namespace Pixsense {


  TrackedFace::TrackedFace() :
   is_copy(false),
   has_face(false),
   was_tracking(false),
   started_tracking_at(std::chrono::system_clock::from_time_t(0)),
   had_face_at(std::chrono::system_clock::from_time_t(0))
  { }

  TrackedFace::TrackedFace(const TrackedFace& copy) :
    is_copy(true),
    face(copy.face),
    has_face(copy.has_face),
    was_tracking(copy.was_tracking),
    started_tracking_at(copy.started_tracking_at),
    had_face_at(copy.had_face_at)
  { }

  bool TrackedFace::is_tracking()
  {
    std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();

    std::chrono::duration<float> since_last_face = now - had_face_at;
    bool tracking = has_face || since_last_face.count() < 1.0f;

    std::chrono::duration<float> since_started_tracking = now - started_tracking_at;
    if (tracking && since_started_tracking.count() > 30.0f) {
      if (!is_copy)
        fprintf(stderr, "Face tracking timed out\n");
      cancel_tracking();
      return false;
    }
    return tracking;
  }

  bool TrackedFace::get_has_face()
  {
    return has_face;
  }

  void TrackedFace::tracking(cv::Rect face)
  {
    if(!is_tracking()) {
      if (!is_copy)
        fprintf(stderr, "Face tracking started\n");
      started_tracking_at = std::chrono::high_resolution_clock::now();
    }
    this->face = face;
    has_face = true;
    had_face_at = std::chrono::high_resolution_clock::now();
    was_tracking = true;
  }

  void TrackedFace::not_tracking()
  {
    has_face = false;
    if (was_tracking && !is_tracking()) {
      if (!is_copy)
        fprintf(stderr, "Face tracking stopped\n");
      cancel_tracking();
    }
  }

  void TrackedFace::cancel_tracking()
  {
      has_face = false;
      was_tracking = false;
      started_tracking_at = std::chrono::system_clock::from_time_t(0);
      had_face_at = std::chrono::system_clock::from_time_t(0);
  }
  RealsenseTracker::RealsenseTracker() : 
   pipe(std::make_shared<rs2::pipeline>(realsense_context)),
   started(false)
  {
    realsense_context.set_devices_changed_callback([&](rs2::event_information& info)
    {
      update_pipe();
    });   
  }

  void RealsenseTracker::tick(AbstractFaceTracker& face_detect, glm::vec3 &face_location) {
    try {
      rs2::frameset unaligned_frames;
      rs2::align align(rs2_stream::RS2_STREAM_COLOR);

      if (started ) {
        unaligned_frames = pipe->wait_for_frames();
        rs2::frameset aligned_frames = align.process(unaligned_frames);
        // rs2::video_frame images = aligned_frames.get_color_frame();
        rs2::depth_frame depths = unaligned_frames.get_depth_frame();
        rs2::video_frame ir_frame = unaligned_frames.get_infrared_frame(1);
        // cv::Mat image_matrix = RealsenseTracker::frame_to_mat(images);
        cv::Mat depth_matrix = RealsenseTracker::frame_to_mat(depths);
        cv::Mat greys_matrix = RealsenseTracker::frame_to_mat(ir_frame);

        cv::Mat frame;
        cvtColor(greys_matrix, frame, cv::COLOR_GRAY2BGR);
        tracked_face = face_detect.detect(frame, depth_matrix);
        if(!tracked_face.is_tracking()) {
          return;
        }
        cv::Rect real_face = cv::Rect(
          tracked_face.face.x , tracked_face.face.y ,
          tracked_face.face.width , tracked_face.face.height);

        float distance = rect_distance(depths, real_face);

        if (distance == 0.0f) {
          return;
        }

        rs2_intrinsics intrin = depths.get_profile().as<rs2::video_stream_profile>().get_intrinsics();

        float pixel[2] = {real_face.x + real_face.width / 2.0f, real_face.y + real_face.height / 2.0f };
        rs2_deproject_pixel_to_point(&face_location[0], &intrin, pixel, distance);
        face_location.y = -face_location.y;
        face_location.x = -face_location.x;
      } else {
        update_pipe();
      }
    } catch(const std::exception& e) {
      fprintf(stderr, "RealsenseTracker Exception: %s\n", e.what());
      started = false;
      update_pipe();
    }
    return;
  }

  void RealsenseTracker::update_pipe() {
    size_t device_count = realsense_context.query_devices().size();
    if(!started && device_count > 0) {
      fprintf(stderr, "RealsenseTracker: starting with %zu devices\n", device_count);
      rs2::config config;
      config.enable_stream(RS2_STREAM_DEPTH, 1280, 720,  RS2_FORMAT_Z16, 30);
      config.enable_stream(RS2_STREAM_COLOR, 1920, 1080, RS2_FORMAT_RGB8, 30);
      config.enable_stream(RS2_STREAM_INFRARED, 1, 1280, 720, RS2_FORMAT_Y8, 30);

      // config.enable_stream(RS2_STREAM_INFRARED, 2);
      // config.enable_stream(RS2_STREAM_DEPTH, 1);
      try {
        pipe->stop();
      } catch(const std::exception& e) {

      }
      pipeline_profile = pipe->start(config);
      rs2::device selected_device = pipeline_profile.get_device();
      auto depth_sensor = selected_device.first<rs2::depth_sensor>();
      if (depth_sensor.supports(RS2_OPTION_EMITTER_ENABLED))
      {
          depth_sensor.set_option(RS2_OPTION_EMITTER_ENABLED, 0.f); // Enable emitter
      }

      started = true;
    } else if (started && device_count == 0) {
      started = false;
      pipe->stop();
    }
  }

  // Convert rs2::frame to cv::Mat
  cv::Mat RealsenseTracker::frame_to_mat(const rs2::frame& f)
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