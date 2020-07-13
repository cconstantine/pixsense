#include <pixsense/mob.hpp>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/string_cast.hpp>

namespace Pixsense {

  Mob::Mob() : leader_id(0), max_id(0), leader_selected_at(now()) { }

  void Mob::update(const std::vector<Pixsense::Person>& visible_people) {
    std::map<unsigned int, Pixsense::Person> existing_people = people;
    std::vector<Pixsense::Person> leftovers;


    for(const Pixsense::Person& person : visible_people) {
      float dist = INFINITY;
      unsigned int closest = max_id;
      for(auto& item : existing_people) {
        float cur_dist = person.avg_distance(item.second);
        if (cur_dist < dist) {
          closest = item.first;
          dist = cur_dist;
        }
      }

      if (dist == INFINITY) {
        people[closest] = person;
        max_id++;
      } else {
        // Check for collision
        if (people[closest].last_seen == person.last_seen) {
          Pixsense::Person prev_match = people[closest];
          Pixsense::Person curr_match = existing_people[closest];

          if(prev_match.avg_distance(curr_match) < curr_match.avg_distance(person)) {
            people[max_id++] = person;
          } else {
            leftovers.push_back(prev_match);
            people[closest] = person;
          }
        } else {
          people[closest] = person;
        }

      }

    }

    if(leftovers.size() > 0) {
      update(leftovers);
    }

  }

  bool Mob::has_leader() {
    Pixsense::Person p;
    return leader(p);
  }

  bool Mob::leader(Pixsense::Person& output) {
    // Cleanup untracked people.
    for(std::map<unsigned int, Pixsense::Person>::const_iterator iter = people.cbegin();iter != people.cend();) {
      std::chrono::duration<float> since_last_seen = now() - iter->second.last_seen;

      if (since_last_seen.count() > 1.0f) {
        people.erase(iter++);
        fprintf(stderr, "Person timed out (%d)\n", people.size());
      } else {
        ++iter;
      }
    }

    std::chrono::duration<float> since_leader_selected = now() - leader_selected_at;
    bool expired = since_leader_selected.count() > 30.0f;

    std::map<unsigned int, Pixsense::Person>::iterator result = people.find(leader_id);
    if((expired || result == people.end()) && people.size() > 0) {
      do {
        result = people.begin();
        std::advance(result, rand() % people.size() );
      } while(result->first == leader_id && people.size() != 1);

      leader_selected_at = now();
      leader_id = result->first;
    }
    output = result->second;

    return result != people.end();
  }

  std::chrono::time_point<std::chrono::high_resolution_clock> Mob::now() {
    return std::chrono::high_resolution_clock::now();
  }
}
