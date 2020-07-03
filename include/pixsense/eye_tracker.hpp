#pragma once
// OpenPose dependencies
#include <openpose/headers.hpp>

#include <pixsense/face_finder.hpp>

namespace Pixsense {
  class Person {
  public:
    Person();
    Person(const Person& p);
    Person(glm::vec2 right_eye, glm::vec2 left_eye);

    glm::vec2 right_eye;
    glm::vec2 left_eye;

    glm::vec2 midpoint();
  };

  class EyeTracker : public Pixsense::AbstractFaceTracker {
  public:
    EyeTracker();

    virtual bool detect(const rs2::frameset & frame, cv::Rect& detection);

    bool should_exit;
  private:
    op::Wrapper opWrapper{op::ThreadManagerMode::Asynchronous};

    Person selected_person;

    cv::Mat previous_frame;
  };

}