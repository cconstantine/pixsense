#include <catch2/catch.hpp>

#include <pixsense/mob.hpp>

class TestMob : public Pixsense::Mob {
public:
  static std::chrono::time_point<std::chrono::high_resolution_clock> test_time;

  std::map<unsigned int, struct record >& get_people();

protected:
  virtual std::chrono::time_point<std::chrono::high_resolution_clock> now() {
    return test_time;
  }

};

std::chrono::time_point<std::chrono::high_resolution_clock> TestMob::test_time = std::chrono::high_resolution_clock::now();

TEST_CASE( "Pixsense::Mob" ) {
  TestMob mob;
  std::chrono::duration<int>one_second(1);

  SECTION("#leader") {
    Pixsense::Person leader;
    REQUIRE(mob.leader(leader) == false);

    SECTION("with people") {
      // First person added is leader
      {
        std::vector<Pixsense::Person> people;
        people.push_back(Pixsense::Person(glm::vec2(10, 10), glm::vec2(12, 10), TestMob::test_time));
        mob.update(people);
      }
      REQUIRE(mob.leader(leader));
      REQUIRE(leader == Pixsense::Person(glm::vec2(10, 10), glm::vec2(12, 10), TestMob::test_time));

      TestMob::test_time += std::chrono::milliseconds(750);

      // First person added is leader after adding another
      {
        std::vector<Pixsense::Person> people;
        people.push_back(Pixsense::Person(glm::vec2(10, 10), glm::vec2(12, 10), TestMob::test_time));
        people.push_back(Pixsense::Person(glm::vec2(11, 10), glm::vec2(13, 10), TestMob::test_time));
        mob.update(people);
      }
      REQUIRE(mob.leader(leader));
      REQUIRE(leader == Pixsense::Person(glm::vec2(10, 10), glm::vec2(12, 10), TestMob::test_time));

      TestMob::test_time += std::chrono::milliseconds(1750);

      // First person goes away and second becomes leader
      {
        std::vector<Pixsense::Person> people;
        people.push_back(Pixsense::Person(glm::vec2(11, 10), glm::vec2(13, 10), TestMob::test_time));
        mob.update(people);
      }
      REQUIRE(mob.leader(leader));
      REQUIRE(leader == Pixsense::Person(glm::vec2(11, 10), glm::vec2(13, 10), TestMob::test_time));

      // Wait a bit over 30s
      TestMob::test_time += std::chrono::milliseconds(30100);

      // both people are there and it switches
      {
        std::vector<Pixsense::Person> people;
        people.push_back(Pixsense::Person(glm::vec2(10, 10), glm::vec2(12, 10), TestMob::test_time));
        people.push_back(Pixsense::Person(glm::vec2(11, 10), glm::vec2(13, 10), TestMob::test_time));
        mob.update(people);
      }
      REQUIRE(mob.people.size() == 2);
      REQUIRE(mob.leader(leader));
      REQUIRE(leader == Pixsense::Person(glm::vec2(10, 10), glm::vec2(12, 10), TestMob::test_time));

    }
  }

  SECTION("#update") {
    SECTION("People timing out") {
      {
        std::vector<Pixsense::Person> people;
        people.push_back(Pixsense::Person(glm::vec2(10, 10), glm::vec2(12, 10), TestMob::test_time));
        people.push_back(Pixsense::Person(glm::vec2(11, 10), glm::vec2(13, 10), TestMob::test_time));
        mob.update(people);
        people.clear();
        REQUIRE(mob.people.size() == 2);

        REQUIRE(people.size() == 0);
        TestMob::test_time += std::chrono::milliseconds(750);

        people.push_back(Pixsense::Person(glm::vec2(11, 10), glm::vec2(13, 10), TestMob::test_time));
        mob.update(people);
        people.clear();
        REQUIRE(mob.people.size() == 2);

        TestMob::test_time += std::chrono::milliseconds(750);

        mob.update(people);
        REQUIRE(mob.people.size() == 1);
      }
    }
    SECTION("performance") {
      std::chrono::time_point<std::chrono::high_resolution_clock> test_start;
      std::chrono::time_point<std::chrono::high_resolution_clock> test_finish;
      {
        std::vector<Pixsense::Person> people;
        for(int i = 0;i < 100;i++) {
          people.push_back(Pixsense::Person(glm::vec2(10+i, 10+i), glm::vec2(12+i, 10+i), TestMob::test_time));
        }
        test_start = std::chrono::high_resolution_clock::now();
        mob.update(people);
        test_finish = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> test_timing = test_finish - test_start;
        REQUIRE(test_timing.count()*1000 < 10.0f);
      }
      {
        std::vector<Pixsense::Person> people;
        for(int i = 100;i > 0;i--) {
          people.push_back(Pixsense::Person(glm::vec2(10+i, 10+i), glm::vec2(12+i, 10+i), TestMob::test_time + one_second));
        }
        test_start = std::chrono::high_resolution_clock::now();
        mob.update(people);
        test_finish = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> test_timing = test_finish - test_start;
        REQUIRE(test_timing.count()*1000 < 6.0f);
      }
      {
        std::vector<Pixsense::Person> people;
        for(int i = 0;i > 100;i++) {
          people.push_back(Pixsense::Person(glm::vec2(10+i, 10+i), glm::vec2(12+i, 10+i), TestMob::test_time + one_second));
        }
        test_start = std::chrono::high_resolution_clock::now();
        mob.update(people);
        test_finish = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> test_timing = test_finish - test_start;
        REQUIRE(test_timing.count()*1000 < 6.0f);
      }
    }
    SECTION("with two people") {

      Pixsense::Person person_a(glm::vec2(10, 10), glm::vec2(12, 10), TestMob::test_time);
      Pixsense::Person person_b(glm::vec2(20, 20), glm::vec2(22, 20), TestMob::test_time);

      {
        std::vector<Pixsense::Person> people;
        people.push_back(person_a);

        mob.update(people);

        REQUIRE(mob.people.size() == 1);
        REQUIRE(mob.people.find(0) != mob.people.end());
        REQUIRE(mob.people.find(0)->second == person_a);

        person_a.last_seen += one_second;
        person_b.last_seen += one_second;
      }
      // Adding person_b and person_a
      {
        std::vector<Pixsense::Person> people;
        people.push_back(person_b);
        people.push_back(person_a);

        mob.update(people);

        REQUIRE(mob.people.size() == 2);
        REQUIRE(mob.people.find(0) != mob.people.end());
        REQUIRE(mob.people.find(0)->second == person_a);

        REQUIRE(mob.people.find(1) != mob.people.end());
        REQUIRE(mob.people.find(1)->second == person_b);

        person_a.last_seen += one_second;
        person_b.last_seen += one_second;
      }
      // Adding person_a and person_b
      {
        std::vector<Pixsense::Person> people;
        people.push_back(person_a);
        people.push_back(person_b);

        mob.update(people);

        REQUIRE(mob.people.size() == 2);
        REQUIRE(mob.people.find(0) != mob.people.end());
        REQUIRE(mob.people.find(0)->second == person_a);

        REQUIRE(mob.people.find(1) != mob.people.end());
        REQUIRE(mob.people.find(1)->second == person_b);

        person_a.last_seen += one_second;
        person_b.last_seen += one_second;
      }
    }
    SECTION("with three people") {
      std::chrono::duration<int>one_second(1);

      Pixsense::Person person_a(glm::vec2(10, 10), glm::vec2(12, 10), TestMob::test_time);
      Pixsense::Person person_b(glm::vec2(20, 20), glm::vec2(22, 20), TestMob::test_time);
      Pixsense::Person person_c(glm::vec2(30, 15), glm::vec2(32, 15), TestMob::test_time);

      {
        std::vector<Pixsense::Person> people;
        people.push_back(person_a);

        mob.update(people);

        REQUIRE(mob.people.size() == 1);
        REQUIRE(mob.people.find(0) != mob.people.end());
        REQUIRE(mob.people.find(0)->second == person_a);

        person_a.last_seen += one_second;
        person_b.last_seen += one_second;
        person_c.last_seen += one_second;
      }
      // Adding person_c, person_b and person_a
      {
        std::vector<Pixsense::Person> people;
        people.push_back(person_b);
        people.push_back(person_c);
        people.push_back(person_a);

        mob.update(people);

        REQUIRE(mob.people.size() == 3);
        REQUIRE(mob.people.find(0) != mob.people.end());
        REQUIRE(mob.people.find(0)->second == person_a);

        REQUIRE(mob.people.find(1) != mob.people.end());
        REQUIRE(mob.people.find(1)->second == person_c);

        REQUIRE(mob.people.find(2) != mob.people.end());
        REQUIRE(mob.people.find(2)->second == person_b);

        person_a.last_seen += one_second;
        person_b.last_seen += one_second;
        person_c.last_seen += one_second;
      }
    }
  }

  // SECTION("#avg_distance") {
  //   Pixsense::Person other_person(glm::vec2(1, 1), glm::vec2(2, 4));

  //   REQUIRE(person.avg_distance(other_person) == 1.f);
  // }
}
