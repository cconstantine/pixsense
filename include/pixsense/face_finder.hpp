#pragma once

#include <librealsense2/rs.hpp>
#include <glm/glm.hpp>

#include <pixsense/tracked_face.hpp>

namespace Pixsense
{
  class AbstractFaceTracker {
  public:
    virtual bool detect(const rs2::frameset & frame, cv::Rect& detection) = 0;

  protected:
    static cv::Mat frame_to_mat(const rs2::frame& f);
  };

  class RealsenseTracker
  {
  public:
    RealsenseTracker();
    bool tick(AbstractFaceTracker& face_detect, glm::vec3 &face_location);

    Pixsense::TrackedFace& tracking();
  private:
    void update_pipe();

    rs2::context realsense_context;
    std::shared_ptr<rs2::pipeline> pipe;
    rs2::pipeline_profile pipeline_profile;

    bool started;
    TrackedFace  tracked_face;
  };
}
