#pragma once

#include <grpc++/grpc++.h>
#include "pixrpc.grpc.pb.h"

#include <thread>
#include <mutex>
#include <condition_variable>

namespace Pixsense
{
  // Logic and data behind the server's behavior.
  class TrackingServiceImpl final : public pixrpc::Tracking::Service {
  public:
    void send_location(float x, float y, float z);

  private:
    grpc::Status location_stream(
      grpc::ServerContext* context,
      const pixrpc::LocationStreamArgs* args,
      grpc::ServerWriter<pixrpc::Location>* stream
    ) override;

    std::mutex m;
    std::condition_variable cv;
    pixrpc::Location current_location;
  };
}