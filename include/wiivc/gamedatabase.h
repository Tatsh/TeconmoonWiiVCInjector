// SPDX-License-Identifier: MIT
#pragma once

#include "types.h"
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace wiivc {

    class GameDatabase {
      public:
        // Get game name from ID
        [[nodiscard]] static std::optional<std::string> getName(std::string_view id);

        // Get IDs for a game name
        [[nodiscard]] static std::vector<std::string> getIds(std::string_view name);

        // Get IDs starting with a prefix
        [[nodiscard]] static std::vector<std::string> getIdsStartingWith(std::string_view idStart);

        // Get alternative IDs for a given ID
        [[nodiscard]] static std::vector<std::string> getAlternativeIds(std::string_view initialId);

        // Load database from embedded resource or file
        static Result<void> loadDatabase(std::optional<std::filesystem::path> path = std::nullopt);

      private:
        static inline std::map<std::string, std::string> s_database;
        static inline bool s_loaded{false};
    };

} // namespace wiivc
