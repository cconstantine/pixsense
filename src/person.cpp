#include <pixsense/person.hpp>

namespace Pixsense {
  Person::Person() { }
  Person::Person(const Person& p) : left_eye(p.left_eye), right_eye(p.right_eye) {  }
  Person::Person(glm::vec2 left_eye, glm::vec2 right_eye) : left_eye(left_eye), right_eye(right_eye) { }

  glm::vec2 Person::midpoint() {
    glm::vec2 sum = left_eye + right_eye;
    return glm::vec2(sum.x / 2.0f, sum.y / 2.0f);
  }
}