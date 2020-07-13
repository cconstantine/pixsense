#pragma once

#include <dlib/config_reader.h>
#include <librealsense2/rs.hpp>
#include <glm/glm.hpp>
#include <map>

#include "opencv2/objdetect/objdetect.hpp"

#include <pixsense/mob.hpp>

namespace Pixsense
{

  class AbstractFaceTracker {
  public:
    virtual bool detect(const rs2::frameset & frame, Pixsense::Mob& mob) = 0;

    static cv::Mat frame_to_mat(const rs2::frame& f);
  };

  struct CameraDetails {
    std::shared_ptr<rs2::pipeline> pipe;
    glm::vec3 offset;
    glm::mat4 rotation;
    cv::Mat previous_frame;
    Pixsense::Mob mob;
  };

  class RealsenseTracker
  {
  public:
    RealsenseTracker(const dlib::config_reader& cr);
    bool tick(AbstractFaceTracker& face_detect, glm::vec3 &face_location);

  private:
    void update_pipe();

    void select_next_pipe();

    rs2::context realsense_context;
    std::map<std::string, struct CameraDetails> pipes;
    std::string selected_pipe;

    bool started;
  };
}
