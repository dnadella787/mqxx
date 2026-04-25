#pragma once

#include "saltpepper/moqt/full_track_name.hpp"

#include <vector>

namespace saltpepper::moqt {

class NamespaceRegistry {
  public:
    [[nodiscard]] auto announce_track(FullTrackName track_name) -> bool;
    [[nodiscard]] auto withdraw_track(const FullTrackName& track_name) -> bool;
    [[nodiscard]] auto tracks_with_prefix(const TrackNamespace& prefix) const
        -> std::vector<FullTrackName>;

  private:
    std::vector<FullTrackName> tracks_;
};

} // namespace saltpepper::moqt
