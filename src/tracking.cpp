// ------------------------------------------------ OpenPose C++ Demo ------------------------------------------------
// This example summarizes all the functionality of the OpenPose library. It can...
    // 1. Read a frames source (images, video, webcam, 3D stereo Flir cameras, etc.).
    // 2. Extract and render body/hand/face/foot keypoint/heatmap/PAF of that image.
    // 3. Save the results on disk.
    // 4. Display the rendered pose.
// If the user wants to learn to use the OpenPose C++ library, we highly recommend to start with the examples in
// `examples/tutorial_api_cpp/`.

// #include <pixsense/tracking_service.hpp>
#include <pixsense/face_finder.hpp>
#include <pixsense/eye_tracker.hpp>

#include <gflags/gflags.h>

int main(int argc, char *argv[])
{

  // Parsing command line flags
  gflags::ParseCommandLineFlags(&argc, &argv, false);

  // std::string server_address("0.0.0.0:50051");
  // Pixsense::TrackingServiceImpl service;
  // grpc::ServerBuilder builder;
  // // Listen on the given address without any authentication mechanism.
  // builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // // Register "service" as the instance through which we'll communicate with
  // // clients. In this case it corresponds to an *synchronous* service.
  // builder.RegisterService(&service);
  // // Finally assemble the server.
  // std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  // std::cout << "Server listening on " << server_address << std::endl;


  Pixsense::RealsenseTracker rt;
  glm::vec3 target;

  // Configure OpenPose
  op::opLog("Configuring OpenPose...", op::Priority::High);
  Pixsense::EyeTracker tracker;

  while(!tracker.should_exit) {
    rt.tick(tracker, target);


  }

  // Return successful message
  return 0;
}
