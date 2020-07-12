#include <pixsense/face_finder.hpp>
#include <pixsense/eye_tracker.hpp>
#include <pixrpc/pixrpc.hpp>
#include <dlib/config_reader.h>

#include <gflags/gflags.h>

DEFINE_string(config_file, "config.txt", "configuration file");


int main(int argc, char *argv[])
{
  // Parsing command line flags
  gflags::ParseCommandLineFlags(&argc, &argv, false);

  dlib::config_reader cr(FLAGS_config_file);

  Pixrpc::Server server(5000);

  Pixsense::RealsenseTracker rt(cr.block("cameras"));
  glm::vec3 target;

  // Configure OpenPose
  op::opLog("Configuring OpenPose...", op::Priority::High);
  Pixsense::EyeTracker tracker;

  struct Pixrpc::Location loc;
  while(!tracker.should_exit) {
    if (rt.tick(tracker, loc.point)) {
      server.send_location(loc);
    } 
  }

  return 0;
}
