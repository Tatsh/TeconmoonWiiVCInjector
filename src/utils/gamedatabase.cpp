// SPDX-License-Identifier: MIT
#include "wiivc/gamedatabase.h"
#include "wiivc/stringutils.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace wiivc {

    std::optional<std::string> GameDatabase::getName(std::string_view id) {
        if (!s_loaded) {
            loadDatabase();
        }
        auto it = s_database.find(std::string(id));
        if (it != s_database.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::vector<std::string> GameDatabase::getIds(std::string_view name) {
        if (!s_loaded) {
            loadDatabase();
        }
        std::vector<std::string> ids;
        std::string nameStr(name);
        for (const auto &[id, dbName] : s_database) {
            if (dbName == nameStr) {
                ids.push_back(id);
            }
        }
        return ids;
    }

    std::vector<std::string> GameDatabase::getIdsStartingWith(std::string_view idStart) {
        if (!s_loaded) {
            loadDatabase();
        }
        std::vector<std::string> ids;
        std::string prefix(idStart);
        for (const auto &[id, name] : s_database) {
            if (id.starts_with(prefix)) {
                ids.push_back(id);
            }
            // Database should be sorted, could optimize here
        }
        return ids;
    }

    std::vector<std::string> GameDatabase::getAlternativeIds(std::string_view initialId) {
        std::vector<std::string> result;
        std::string id(initialId);

        // Try original
        result.push_back(id);

        // Try region variants
        if (id.size() >= 4) {
            auto idE = utils::replaceAt(id, 3, 'E'); // English/NTSC-U
            auto idP = utils::replaceAt(id, 3, 'P'); // PAL
            if (std::find(result.begin(), result.end(), idE) == result.end()) {
                result.push_back(idE);
            }
            if (std::find(result.begin(), result.end(), idP) == result.end()) {
                result.push_back(idP);
            }
        }

        // Try IDs for the same game name
        auto name = getName(id);
        if (name) {
            auto ids = getIds(*name);
            for (const auto &altId : ids) {
                if (std::find(result.begin(), result.end(), altId) == result.end()) {
                    result.push_back(altId);
                }
            }
        }

        // Try prefix match (first 3 characters)
        if (id.size() >= 3) {
            auto prefixIds = getIdsStartingWith(id.substr(0, 3));
            for (const auto &prefixId : prefixIds) {
                if (std::find(result.begin(), result.end(), prefixId) == result.end()) {
                    result.push_back(prefixId);
                }
            }
        }

        return result;
    }

    Result<void> GameDatabase::loadDatabase(std::optional<std::filesystem::path> path) {
        // For now, create a minimal embedded database
        // In full implementation, would load from embedded resource or file
        s_database.clear();

        // Sample entries (would be loaded from wiitdb.txt resource)
        s_database["RMCE01"] = "Mario Kart Wii";
        s_database["RMCP01"] = "Mario Kart Wii";
        s_database["RMCJ01"] = "Mario Kart Wii";
        s_database["RSBE01"] = "Super Smash Bros. Brawl";
        s_database["RSBP01"] = "Super Smash Bros. Brawl";
        s_database["RSBJ01"] = "Super Smash Bros. Brawl";

        s_loaded = true;
        return {};
    }

} // namespace wiivc
