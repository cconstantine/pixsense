#pragma once
#include <chrono>

#include "opencv2/objdetect/objdetect.hpp"

namespace Pixsense
{
  class TrackedFace {
  public:
    TrackedFace();

    void tracking(cv::Rect face);
    void not_tracking();
    bool is_tracking();
    bool get_has_face();
    void cancel_tracking();

    cv::Rect face;

  private:
    bool was_tracking;
    bool has_face;
    std::chrono::time_point<std::chrono::high_resolution_clock> had_face_at;
    std::chrono::time_point<std::chrono::high_resolution_clock> started_tracking_at;
  };
}
