#pragma once

#include <glm/glm.hpp>

namespace Pixsense {
  class Person {
  public:
    Person();
    Person(const Person& p);
    Person(glm::vec2 right_eye, glm::vec2 left_eye);

    glm::vec2 right_eye;
    glm::vec2 left_eye;

    glm::vec2 midpoint();
  };
}