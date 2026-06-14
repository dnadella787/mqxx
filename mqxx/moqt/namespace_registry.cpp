#include "mqxx/moqt/namespace_registry.hpp"

#include <utility>

namespace mqxx::moqt {

std::expected<namespace_registry::cref_entry, namespace_registry_error>
namespace_registry::insert(track_namespace name_space) {
    if (find_exact(name_space).has_value()) {
        return std::unexpected(namespace_registry_error::duplicate_namespace);
    }

    entries_.push_back(entry{std::move(name_space)});
    return entries_.back();
}

std::optional<namespace_registry::cref_entry>
namespace_registry::find_exact(const track_namespace& name_space) const {
    for (const entry& stored_entry : entries_) {
        if (stored_entry.ns == name_space) {
            return stored_entry;
        }
    }

    return std::nullopt;
}

std::optional<namespace_registry::cref_entry>
namespace_registry::find_longest_prefix(const track_namespace& ns) const {
    std::optional<cref_entry> best_match = std::nullopt;
    std::size_t best_field_count = 0;

    for (const entry& stored_entry : entries_) {
        // check if this entry is a prefix of ns by evaluating the length
        // of the fields of the track ns and make sure that all N fields in
        // the stored entry are equal to the first N fields of the ns passed
        if (!is_namespace_prefix(stored_entry.ns, ns)) {
            continue;
        }

        // if it is a prefix, we loop until the end constantly setting the best match
        // maybe use a trie instead of a vector
        const std::size_t field_count = stored_entry.ns.fields().size();
        if (!best_match || field_count > best_field_count) {
            best_match = stored_entry;
            best_field_count = field_count;
        }
    }

    return best_match;
}

bool namespace_registry::empty() const noexcept {
    return entries_.empty();
}

std::size_t namespace_registry::size() const noexcept {
    return entries_.size();
}

bool namespace_registry::is_namespace_prefix(const track_namespace& prefix,
                                             const track_namespace& ns) {
    const auto prefix_fields = prefix.fields();
    const auto namespace_fields = ns.fields();
    if (prefix_fields.size() > namespace_fields.size()) {
        return false;
    }

    for (std::size_t i = 0; i < prefix_fields.size(); ++i) {
        if (prefix_fields[i] != namespace_fields[i]) {
            return false;
        }
    }

    return true;
}

} // namespace mqxx::moqt
