#pragma once
#include <openpose/headers.hpp>

#include <pixsense/face_finder.hpp>
#include <pixsense/person.hpp>
#include <pixsense/mob.hpp>

namespace Pixsense {
  class EyeTracker : public Pixsense::AbstractFaceTracker {
  public:
    EyeTracker();

    virtual bool detect(const rs2::frameset & frame, cv::Rect& detection);

    bool should_exit;
  private:
    op::Wrapper opWrapper{op::ThreadManagerMode::Asynchronous};

    Pixsense::Mob mob;

    unsigned int selected_person;

    cv::Mat previous_frame;
  };
}
