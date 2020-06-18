#pragma once
// OpenPose dependencies
#include <openpose/headers.hpp>

#include <pixsense/face_finder.hpp>

namespace Pixsense {
  class EyeTracker : public Pixsense::AbstractFaceTracker {
  public:
    bool should_exit;

    EyeTracker();

    Pixsense::TrackedFace detect(const cv::Mat& frame, const cv::Mat& depth_frame);

    op::Wrapper opWrapper{op::ThreadManagerMode::Asynchronous};
  };
}