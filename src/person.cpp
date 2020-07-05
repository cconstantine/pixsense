#include <pixsense/person.hpp>

namespace Pixsense {
  Person::Person() { }
  Person::Person(const Person& p) :
   left_eye(p.left_eye), right_eye(p.right_eye), last_seen(p.last_seen)  {  }
  Person::Person(glm::vec2 left_eye, glm::vec2 right_eye, std::chrono::time_point<std::chrono::high_resolution_clock> last_seen) :
   left_eye(left_eye),   right_eye(right_eye),   last_seen(last_seen) { }

  glm::vec2 Person::midpoint() const {
    glm::vec2 sum = left_eye + right_eye;
    return glm::vec2(sum.x / 2.0f, sum.y / 2.0f);
  }

  float Person::avg_distance(const Person& other) const {
    return (glm::distance(left_eye, other.left_eye) + glm::distance(right_eye, other.right_eye)) / 2.0f;
  }

  bool Person::operator==(const Person& p) const {
    return p.left_eye == left_eye && p.right_eye == right_eye;
  }
}
