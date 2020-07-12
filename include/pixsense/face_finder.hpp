#pragma once

#include <librealsense2/rs.hpp>
#include <glm/glm.hpp>
#include <map>

#include "opencv2/objdetect/objdetect.hpp"

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

  private:
    void update_pipe();

    void select_next_pipe();

    rs2::context realsense_context;
    std::map<std::string, std::shared_ptr<rs2::pipeline>> pipes;
    std::string selected_pipe;

    bool started;
  };
}
