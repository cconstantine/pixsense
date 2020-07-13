#pragma once

#include <map>
#include <vector>
#include <chrono>

#include <pixsense/person.hpp>

namespace Pixsense {
  class Mob {
  public:
    Mob();

    void update(const std::vector<Pixsense::Person>& visible_people);
    bool leader(Pixsense::Person& output);
    bool has_leader();

    std::map<unsigned int, Pixsense::Person> people;
  protected:
    virtual std::chrono::time_point<std::chrono::high_resolution_clock> now();

    unsigned int leader_id;
    unsigned int max_id;
    std::chrono::time_point<std::chrono::high_resolution_clock> leader_selected_at;
  };
}