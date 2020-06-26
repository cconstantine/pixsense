#include <pixsense/tracking_service.hpp>
#include "pixrpc.grpc.pb.h"

namespace Pixsense
{

  void TrackingServiceImpl::send_location(const glm::vec3& loc) {

    {
      std::lock_guard<std::mutex> lk(m);

      current_location.set_x(loc.x);
      current_location.set_y(loc.y);
      current_location.set_z(loc.z);
    }

    cv.notify_all();
  }

  grpc::Status TrackingServiceImpl::location_stream(
    grpc::ServerContext* context,
    const pixrpc::LocationStreamArgs* args,
    grpc::ServerWriter<pixrpc::Location>* stream
  ) {
    while(true) {
      std::unique_lock<std::mutex> lk(m);
      cv.wait(lk);

      stream->Write(current_location);
    }
    return grpc::Status::OK;
  }

}