#include "mqxx/moqt/namespace_registry.hpp"

#include <algorithm>
#include <ranges>
#include <utility>

namespace mqxx::moqt {

auto namespace_registry::announce_track(full_track_name track_name) -> bool {
    if (std::ranges::find(tracks_, track_name) != tracks_.end()) {
        return false;
    }

    tracks_.push_back(std::move(track_name));
    std::ranges::sort(tracks_, [](const full_track_name& left, const full_track_name& right) {
        return render_serialized_full_track_name(left) < render_serialized_full_track_name(right);
    });

    return true;
}

auto namespace_registry::withdraw_track(const full_track_name& track_name) -> bool {
    const auto existing = std::ranges::find(tracks_, track_name);
    if (existing == tracks_.end()) {
        return false;
    }

    tracks_.erase(existing);
    return true;
}

auto namespace_registry::tracks_with_prefix(const track_namespace& prefix) const
    -> std::vector<full_track_name> {
    std::vector<full_track_name> matches;
    for (const full_track_name& track : tracks_) {
        if (namespace_starts_with(track.track_namespace, prefix)) {
            matches.push_back(track);
        }
    }

    return matches;
}

} // namespace mqxx::moqt
