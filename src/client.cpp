#include <pixrpc/pixrpc.hpp>

using namespace std;
int main(int argc, char *argv[])
{
  Pixrpc::Client client("127.0.0.1", 5000);

  while(true) {
    struct Pixrpc::Location loc;
    client.receive_location(loc);

    fprintf(stderr, "%f, %f, %f\n", loc.x, loc.y, loc.z);
  }

  // Return successful message
  // server->Wait();
  return 0;
}
