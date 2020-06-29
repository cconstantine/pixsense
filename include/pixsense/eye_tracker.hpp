#pragma once
// OpenPose dependencies
#include <openpose/headers.hpp>

#include <pixsense/face_finder.hpp>

namespace Pixsense {
  struct person {
    glm::vec2 right_eye;
    glm::vec2 left_eye;
    glm::vec2 nose;
  };

  class EyeTracker : public Pixsense::AbstractFaceTracker {
  public:
    EyeTracker();

    virtual bool detect(const rs2::frameset & frame, cv::Rect& detection);

    bool should_exit;
  private:
    op::Wrapper opWrapper{op::ThreadManagerMode::Asynchronous};

    struct person selected_person;

    cv::Mat previous_frame;
  };

}