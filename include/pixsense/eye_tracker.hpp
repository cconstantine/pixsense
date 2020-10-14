#pragma once
#include <openpose/headers.hpp>

#include <pixsense/face_finder.hpp>
#include <pixsense/person.hpp>
#include <pixsense/mob.hpp>

namespace Pixsense {
  class EyeTracker : public Pixsense::AbstractFaceTracker {
  public:
    EyeTracker();

    virtual bool detect(const rs2::frameset & frame, Pixsense::Mob& mob);

  private:
    op::Wrapper opWrapper{op::ThreadManagerMode::Asynchronous};
  };
}
