#include<pixsense/tracked_face.hpp>

namespace Pixsense {
  TrackedFace::TrackedFace() :
   has_face(false),
   was_tracking(false),
   started_tracking_at(std::chrono::system_clock::from_time_t(0)),
   had_face_at(std::chrono::system_clock::from_time_t(0))
  { }

  bool TrackedFace::is_tracking()
  {
    std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();

    std::chrono::duration<float> since_last_face = now - had_face_at;
    bool tracking = has_face || since_last_face.count() < 1.0f;

    std::chrono::duration<float> since_started_tracking = now - started_tracking_at;
    if (tracking && since_started_tracking.count() > 30.0f) {
      fprintf(stderr, "Face tracking timed out\n");
      cancel_tracking();
      return false;
    }
    return tracking;
  }

  bool TrackedFace::get_has_face()
  {
    return has_face;
  }

  void TrackedFace::tracking(cv::Rect face)
  {
    if(!is_tracking()) {
      fprintf(stderr, "Face tracking started\n");
      started_tracking_at = std::chrono::high_resolution_clock::now();
    }
    this->face = face;
    has_face = true;
    had_face_at = std::chrono::high_resolution_clock::now();
    was_tracking = true;
  }

  void TrackedFace::not_tracking()
  {
    has_face = false;
    if (was_tracking && !is_tracking()) {
      fprintf(stderr, "Face tracking stopped\n");
      cancel_tracking();
    }
  }

  void TrackedFace::cancel_tracking()
  {
      has_face = false;
      was_tracking = false;
      started_tracking_at = std::chrono::system_clock::from_time_t(0);
      had_face_at = std::chrono::system_clock::from_time_t(0);
  }
}
