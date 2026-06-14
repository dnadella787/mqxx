#pragma once

#include "mqxx/moqt/track_identity.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <optional>
#include <vector>

namespace mqxx::moqt {

enum class namespace_registry_error : std::uint8_t {
    duplicate_namespace,
};

class namespace_registry {
  public:
    struct entry {
        track_namespace ns;
    };

    using cref_entry = std::reference_wrapper<const entry>;

    [[nodiscard]] std::expected<cref_entry, namespace_registry_error>
    insert(track_namespace name_space);

    [[nodiscard]] std::optional<cref_entry> find_exact(const track_namespace& name_space) const;

    [[nodiscard]] std::optional<cref_entry> find_longest_prefix(const track_namespace& ns) const;

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

  private:
    [[nodiscard]] static bool is_namespace_prefix(const track_namespace& prefix,
                                                  const track_namespace& ns);

    std::vector<entry> entries_;
};

} // namespace mqxx::moqt
