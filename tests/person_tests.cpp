#include <catch2/catch.hpp>

#include <pixsense/person.hpp>


TEST_CASE( "Pixsense::Person" ) {
  std::chrono::time_point<std::chrono::high_resolution_clock> test_time = std::chrono::high_resolution_clock::now();

  Pixsense::Person person(glm::vec2(1, 1), glm::vec2(2, 2), test_time);

  SECTION("Person(const Person& p)") {
    Pixsense::Person other_person = person;

    REQUIRE(person.left_eye  == other_person.left_eye);
    REQUIRE(person.right_eye == other_person.right_eye);
    REQUIRE(person.last_seen == other_person.last_seen);
  }

  SECTION("#midpoint") {
    REQUIRE(person.midpoint() == glm::vec2(1.5, 1.5));
  }

  SECTION("#avg_distance") {
    Pixsense::Person other_person(glm::vec2(1, 1), glm::vec2(2, 4), test_time);

    REQUIRE(person.avg_distance(other_person) == 1.f);
  }
}
