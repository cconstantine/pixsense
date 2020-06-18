#pragma once

#include <librealsense2/rs.hpp>
#include <memory>
#include <glm/glm.hpp>
#include "opencv2/objdetect/objdetect.hpp"

namespace Pixsense
{

  class TrackedFace {
  public:
    TrackedFace();
    TrackedFace(const TrackedFace& copy);

    void tracking(cv::Rect face);
    void not_tracking();
    bool is_tracking();
    bool get_has_face();
    void cancel_tracking();

    cv::Rect face;

  private:
    bool is_copy;
    bool was_tracking;
    bool has_face;
    std::chrono::time_point<std::chrono::high_resolution_clock> had_face_at;
    std::chrono::time_point<std::chrono::high_resolution_clock> started_tracking_at;
  };

  class AbstractFaceTracker {
  public:
    virtual TrackedFace detect(const cv::Mat& frame, const cv::Mat& depth_frame) = 0;
  };

  class RealsenseTracker
  {
  public:
    RealsenseTracker();
    void tick(AbstractFaceTracker& face_detect, glm::vec3 &face_location);

    TrackedFace  tracked_face;

  private:
    void update_pipe();

    rs2::context realsense_context;
    std::shared_ptr<rs2::pipeline> pipe;
    rs2::pipeline_profile pipeline_profile;

    bool started;

    static cv::Mat frame_to_mat(const rs2::frame& f);
  };
}
