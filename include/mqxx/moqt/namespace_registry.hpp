#pragma once

#include "mqxx/moqt/full_track_name.hpp"

#include <vector>

namespace mqxx::moqt {

class namespace_registry {
  public:
    [[nodiscard]] auto announce_track(full_track_name track_name) -> bool;
    [[nodiscard]] auto withdraw_track(const full_track_name& track_name) -> bool;
    [[nodiscard]] auto tracks_with_prefix(const track_namespace& prefix) const
        -> std::vector<full_track_name>;

  private:
    std::vector<full_track_name> tracks_;
};

} // namespace mqxx::moqt
