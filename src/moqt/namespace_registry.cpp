#include "saltpepper/moqt/namespace_registry.hpp"

#include <algorithm>

namespace saltpepper::moqt {

auto NamespaceRegistry::announce_track(FullTrackName track_name) -> bool {
    const auto existing = std::find(tracks_.begin(), tracks_.end(), track_name);
    if (existing != tracks_.end()) {
        return false;
    }

    tracks_.push_back(std::move(track_name));
    std::sort(tracks_.begin(), tracks_.end(),
              [](const FullTrackName& left, const FullTrackName& right) {
                  return render_serialized_full_track_name(left) <
                         render_serialized_full_track_name(right);
              });

    return true;
}

auto NamespaceRegistry::withdraw_track(const FullTrackName& track_name) -> bool {
    const auto existing = std::find(tracks_.begin(), tracks_.end(), track_name);
    if (existing == tracks_.end()) {
        return false;
    }

    tracks_.erase(existing);
    return true;
}

auto NamespaceRegistry::tracks_with_prefix(const TrackNamespace& prefix) const
    -> std::vector<FullTrackName> {
    std::vector<FullTrackName> matches;
    for (const FullTrackName& track : tracks_) {
        if (namespace_starts_with(track.track_namespace, prefix)) {
            matches.push_back(track);
        }
    }

    return matches;
}

} // namespace saltpepper::moqt
