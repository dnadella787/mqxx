#pragma once

#include "mqxx/moqt/full_track_name.hpp"

#include <vector>

namespace mqxx::moqt {

class namespace_registry {
  public:
    [[nodiscard]] bool announce_track(full_track_name track_name);
    [[nodiscard]] bool withdraw_track(const full_track_name& track_name);
    [[nodiscard]] std::vector<full_track_name> tracks_with_prefix(const track_namespace& prefix) const;

  private:
    std::vector<full_track_name> tracks_;
};

} // namespace mqxx::moqt
