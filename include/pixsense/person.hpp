#pragma once

#include <glm/glm.hpp>
#include <chrono>

namespace Pixsense {
  class Person {
  public:
    Person();
    Person(const Person& p);
    Person(glm::vec2 right_eye, glm::vec2 left_eye, std::chrono::time_point<std::chrono::high_resolution_clock> last_seen);


    glm::vec2 midpoint() const;
    float     avg_distance(const Person& other) const;

    bool operator==(const Person& p) const;

    std::chrono::time_point<std::chrono::high_resolution_clock> last_seen;
    glm::vec2 right_eye;
    glm::vec2 left_eye;
  };
}