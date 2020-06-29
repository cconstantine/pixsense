// ------------------------------------------------ OpenPose C++ Demo ------------------------------------------------
// This example summarizes all the functionality of the OpenPose library. It can...
    // 1. Read a frames source (images, video, webcam, 3D stereo Flir cameras, etc.).
    // 2. Extract and render body/hand/face/foot keypoint/heatmap/PAF of that image.
    // 3. Save the results on disk.
    // 4. Display the rendered pose.
// If the user wants to learn to use the OpenPose C++ library, we highly recommend to start with the examples in
// `examples/tutorial_api_cpp/`.

#include <pixsense/face_finder.hpp>
#include <pixsense/eye_tracker.hpp>
#include <pixrpc/pixrpc.hpp>

#include <gflags/gflags.h>

using namespace std;
int main(int argc, char *argv[])
{
  // Parsing command line flags
  gflags::ParseCommandLineFlags(&argc, &argv, false);

  Pixrpc::Server server(5000);

  Pixsense::RealsenseTracker rt;
  glm::vec3 target;

  // Configure OpenPose
  op::opLog("Configuring OpenPose...", op::Priority::High);
  Pixsense::EyeTracker tracker;

  while(!tracker.should_exit) {
    if (rt.tick(tracker, target)) {
      struct Pixrpc::Location loc = {target.x, target.y, target.z};
      server.send_location(loc);
    } 
  }

  // Return successful message
  // server->Wait();
  return 0;
}
