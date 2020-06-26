#pragma once
// OpenPose dependencies
#include <openpose/headers.hpp>

#include <pixsense/face_finder.hpp>

namespace Pixsense {
  class EyeTracker : public Pixsense::AbstractFaceTracker {
  public:
    bool should_exit;

    EyeTracker();

    virtual bool detect(const rs2::frameset & frame, cv::Rect& detection);

    op::Wrapper opWrapper{op::ThreadManagerMode::Asynchronous};

  };
}