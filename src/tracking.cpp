#include <pixsense/face_finder.hpp>
#include <pixsense/eye_tracker.hpp>
#include <pixpq/pixpq.hpp>
#include <dlib/config_reader.h>

#include <gflags/gflags.h>

DEFINE_string(config_file, "config.txt", "configuration file");


int main(int argc, char *argv[])
{
  // Parsing command line flags
  gflags::ParseCommandLineFlags(&argc, &argv, false);

  dlib::config_reader cr(FLAGS_config_file);

  pixpq::db db;

  Pixsense::RealsenseTracker rt(cr.block("cameras"));
  glm::vec3 target;

  // Configure OpenPose
  op::opLog("Configuring OpenPose...", op::Priority::High);
  Pixsense::EyeTracker tracker;
  while(!tracker.should_exit) {
    glm::vec3 point;
    if (rt.tick(tracker, point)) {
      pixpq::location loc(point.x, point.y, point.z);
      db.send("pixo-16.local", loc);
    } 
  }

  return 0;
}
