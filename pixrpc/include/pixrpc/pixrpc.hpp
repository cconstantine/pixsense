#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

#include <dlib/bridge.h>
#include <dlib/type_safe_union.h>

#include <glm/glm.hpp>

namespace Pixrpc {
  struct Location {
    glm::vec3 point;
  };

  void serialize (const struct Pixrpc::Location& item, std::ostream& out);
  void deserialize (struct Pixrpc::Location& item, std::istream& in);

  class Server {
  public:
    Server(unsigned short port);
    void send_location(struct Pixrpc::Location& loc);

  private:
    dlib::pipe<struct Location> in, out;
    dlib::bridge bridge;
  };

  class Client {
  public:
    Client(const std::string& ip, unsigned short port);

    void receive_location(struct Pixrpc::Location& loc);
  private:
    dlib::pipe<struct Location> in, out;
    dlib::bridge bridge;
  };
}
