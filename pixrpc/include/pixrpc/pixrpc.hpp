#pragma once

// dlib::pipe<int> in(4), out(4);


// // This bridge will listen on port 12345 for an incoming TCP connection.  Then
// // it will read data from that connection and put it into the in pipe.
// bridge b2(listen_on_port(12345), receive(in));

// // This bridge will initiate a TCP connection and then start dequeuing 
// // objects from out and transmitting them over the connection.
// bridge b1(connect_to_ip_and_port("127.0.0.1", 12345), transmit(out));

// // As an aside, in a real program, each of these bridges and pipes would be in a 
// // separate application.  But to make this example self contained they are both 
// // right here.



// // Now let's put some things into the out pipe
// int value = 1;
// out.enqueue(value);

// value = 2;
// out.enqueue(value);

// value = 3;
// out.enqueue(value);


// // Now those 3 ints can be dequeued from the in pipe.  They will show up
// // in the same order they were inserted into the out pipe.
// in.dequeue(value);
// cout << "dequeued value: "<< value << endl;
// in.dequeue(value);
// cout << "dequeued value: "<< value << endl;
// in.dequeue(value);
// cout << "dequeued value: "<< value << endl;

namespace Pixsense {
  class Server {

  }

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