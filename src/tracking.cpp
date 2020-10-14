#include <pixsense/face_finder.hpp>
#include <pixsense/eye_tracker.hpp>
#include <pixrpc/pixrpc.hpp>
#include <dlib/config_reader.h>
#include <gflags/gflags.h>
#include <opencv2/highgui.hpp>

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
  float offset = 0.01f;

  bool should_exit = false;
  struct Pixrpc::Location loc;
  while(!should_exit) {
    const char key = (char)cv::waitKey(1);
    if (key == 27) {
      fprintf(stderr, "exit\n");
      should_exit = true;
    } else if (key == 82) {
      rt.shift(glm::vec3(0.0f, offset, 0.0f));
    } else if (key == 84) {
      rt.shift(glm::vec3(0.0f, -offset, 0.0f));
    } else if (key == 83) {
      rt.shift(glm::vec3(offset, 0.0f, 0.0f));
    } else if (key == 81) {
      rt.shift(glm::vec3(-offset, 0.0f, 0.0f));
    } else if (key == 46) {
      rt.shift(glm::vec3(0.0f, 0.0f, offset));
    } else if (key == 44) {
      rt.shift(glm::vec3(0.0f, 0.0f, -offset));
    } else if (key != -1) {
      fprintf(stderr, "key: %d\n", key);
    }
  
    if (rt.tick(tracker, loc.point)) {
      server.send_location(loc);
    } 
  }

  return 0;
}
