module;

#include <algorithm>
#include <ranges>
#include <vector>

export module mqxx.moqt.namespace_registry;

export import mqxx.moqt.full_track_name;

export namespace mqxx::moqt {

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
