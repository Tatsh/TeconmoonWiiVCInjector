// SPDX-License-Identifier: MIT
#include "wiivc/xmlgen.h"
#include <fstream>
#include <pugixml.hpp>
#include <sstream>

namespace wiivc::xmlgen {

    Result<std::string> generateAppXML(const AppXMLConfig &config) {
        pugi::xml_document doc;

        // Add XML declaration
        auto decl = doc.prepend_child(pugi::node_declaration);
        decl.append_attribute("version") = "1.0";
        decl.append_attribute("encoding") = "utf-8";

        // Create root app element
        auto app = doc.append_child("app");
        app.append_attribute("type") = "complex";
        app.append_attribute("access") = "777";

        // Add version
        auto version = app.append_child("version");
        version.append_attribute("type") = "unsignedInt";
        version.append_attribute("length") = "4";
        version.text() = "16";

        // Add os_version
        auto osVersion = app.append_child("os_version");
        osVersion.append_attribute("type") = "hexBinary";
        osVersion.append_attribute("length") = "8";
        osVersion.text() = config.osVersion.c_str();

        // Add title_id
        auto titleId = app.append_child("title_id");
        titleId.append_attribute("type") = "hexBinary";
        titleId.append_attribute("length") = "8";
        titleId.text() = config.titleId.c_str();

        // Add title_version
        auto titleVersion = app.append_child("title_version");
        titleVersion.append_attribute("type") = "hexBinary";
        titleVersion.append_attribute("length") = "2";
        titleVersion.text() = "0000";

        // Add sdk_version
        auto sdkVersion = app.append_child("sdk_version");
        sdkVersion.append_attribute("type") = "unsignedInt";
        sdkVersion.append_attribute("length") = "4";
        sdkVersion.text() = std::to_string(config.sdkVersion).c_str();

        // Add app_type
        auto appType = app.append_child("app_type");
        appType.append_attribute("type") = "hexBinary";
        appType.append_attribute("length") = "4";
        appType.text() = "8000002E";

        // Add group_id
        auto groupId = app.append_child("group_id");
        groupId.append_attribute("type") = "hexBinary";
        groupId.append_attribute("length") = "4";
        groupId.text() = config.titleIdHex.c_str();

        // Add os_mask
        auto osMask = app.append_child("os_mask");
        osMask.append_attribute("type") = "hexBinary";
        osMask.append_attribute("length") = "32";
        osMask.text() = "0000000000000000000000000000000000000000000000000000000000000000";

        // Add common_id
        auto commonId = app.append_child("common_id");
        commonId.append_attribute("type") = "hexBinary";
        commonId.append_attribute("length") = "8";
        commonId.text() = "0000000000000000";

        // Convert to string
        std::ostringstream oss;
        doc.save(oss, "  ");
        return oss.str();
    }

    Result<std::string> generateMetaXML(const MetaXMLConfig &config) {
        pugi::xml_document doc;

        // Add XML declaration
        auto decl = doc.prepend_child(pugi::node_declaration);
        decl.append_attribute("version") = "1.0";
        decl.append_attribute("encoding") = "utf-8";

        // Create root menu element
        auto menu = doc.append_child("menu");
        menu.append_attribute("type") = "complex";
        menu.append_attribute("access") = "777";

        // Helper lambda to add simple elements
        auto addElement = [&menu](const char *name,
                                   const char *type,
                                   const char *length,
                                   const char *value) {
            auto elem = menu.append_child(name);
            elem.append_attribute("type") = type;
            elem.append_attribute("length") = length;
            elem.text() = value;
        };

        // Add all required meta.xml fields
        addElement("version", "unsignedInt", "4", "33");
        addElement("product_code", "string", "32", config.productCode.c_str());
        addElement("content_platform", "string", "32", "WUP");
        addElement("company_code", "string", "8", "0001");
        addElement("mastering_date", "string", "32", "");
        addElement("logo_type", "unsignedInt", "4", "0");
        addElement("app_launch_type", "hexBinary", "4", "00000000");
        addElement("invisible_flag", "hexBinary", "4", "00000000");
        addElement("no_managed_flag", "hexBinary", "4", "00000000");
        addElement("no_event_log", "hexBinary", "4", "00000002");
        addElement("no_icon_database", "hexBinary", "4", "00000000");
        addElement("launching_flag", "hexBinary", "4", "00000004");
        addElement("install_flag", "hexBinary", "4", "00000000");
        addElement("closing_msg", "unsignedInt", "4", "0");
        addElement("title_version", "unsignedInt", "4", "0");
        addElement("title_id", "hexBinary", "8", config.titleId.c_str());
        addElement("group_id", "hexBinary", "4", config.titleIdHex.c_str());
        addElement("boss_id", "hexBinary", "8", "0000000000000000");
        addElement("os_version", "hexBinary", "8", "000500101000400A");
        addElement("app_size", "hexBinary", "8", "0000000000000000");
        addElement("common_save_size", "hexBinary", "8", "0000000000000000");
        addElement("account_save_size", "hexBinary", "8", "0000000000000000");
        addElement("common_boss_size", "hexBinary", "8", "0000000000000000");
        addElement("account_boss_size", "hexBinary", "8", "0000000000000000");
        addElement("save_no_rollback", "unsignedInt", "4", "0");
        addElement("join_game_id", "hexBinary", "4", "00000000");
        addElement("join_game_mode_mask", "hexBinary", "8", "0000000000000000");
        addElement("bg_daemon_enable", "unsignedInt", "4", "0");
        addElement("olv_accesskey", "unsignedInt", "4", "3921400692");
        addElement("wood_tin", "unsignedInt", "4", "0");
        addElement("e_manual", "unsignedInt", "4", "0");
        addElement("e_manual_version", "unsignedInt", "4", "0");
        addElement("region", "hexBinary", "4", "00000002");

        // Parental control ratings
        addElement("pc_cero", "unsignedInt", "4", "128");
        addElement("pc_esrb", "unsignedInt", "4", "6");
        addElement("pc_bbfc", "unsignedInt", "4", "192");
        addElement("pc_usk", "unsignedInt", "4", "128");
        addElement("pc_pegi_gen", "unsignedInt", "4", "128");
        addElement("pc_pegi_fin", "unsignedInt", "4", "192");
        addElement("pc_pegi_prt", "unsignedInt", "4", "128");
        addElement("pc_pegi_bbfc", "unsignedInt", "4", "128");
        addElement("pc_cob", "unsignedInt", "4", "128");
        addElement("pc_grb", "unsignedInt", "4", "128");
        addElement("pc_cgsrr", "unsignedInt", "4", "128");
        addElement("pc_oflc", "unsignedInt", "4", "128");
        addElement("pc_reserved0", "unsignedInt", "4", "192");
        addElement("pc_reserved1", "unsignedInt", "4", "192");
        addElement("pc_reserved2", "unsignedInt", "4", "192");
        addElement("pc_reserved3", "unsignedInt", "4", "192");

        // External device support
        addElement("ext_dev_nunchaku", "unsignedInt", "4", "0");
        addElement("ext_dev_classic", "unsignedInt", "4", "0");
        addElement("ext_dev_urcc", "unsignedInt", "4", "0");
        addElement("ext_dev_board", "unsignedInt", "4", "0");
        addElement("ext_dev_usb_keyboard", "unsignedInt", "4", "0");
        addElement("ext_dev_etc", "unsignedInt", "4", "0");
        addElement("ext_dev_etc_name", "string", "512", "");
        addElement("eula_version", "unsignedInt", "4", "0");

        // DRC use
        addElement("drc_use", "unsignedInt", "4", std::to_string(config.drcUse).c_str());

        addElement("network_use", "unsignedInt", "4", "0");
        addElement("online_account_use", "unsignedInt", "4", "0");
        addElement("direct_boot", "unsignedInt", "4", "0");

        // Reserved flags
        addElement("reserved_flag0", "hexBinary", "4", "00010001");
        addElement("reserved_flag1", "hexBinary", "4", "00080023");
        addElement("reserved_flag2", "hexBinary", "4", config.titleIdHex.c_str());
        addElement("reserved_flag3", "hexBinary", "4", "00000000");
        addElement("reserved_flag4", "hexBinary", "4", "00000000");
        addElement("reserved_flag5", "hexBinary", "4", "00000000");
        addElement("reserved_flag6", "hexBinary", "4", "00000003");
        addElement("reserved_flag7", "hexBinary", "4", "00000005");

        // Add localized names for all languages
        const char *languages[] = {"ja", "en", "fr", "de", "it", "es", "zhs", "ko", "nl", "pt", "ru", "zht"};
        
        for (const char *lang : languages) {
            std::string longNameKey = std::string("longname_") + lang;
            std::string shortNameKey = std::string("shortname_") + lang;
            std::string publisherKey = std::string("publisher_") + lang;

            if (config.enableLine2) {
                auto longName = menu.append_child(longNameKey.c_str());
                longName.append_attribute("type") = "string";
                longName.append_attribute("length") = "512";
                longName.text() = (config.longName + "\n" + config.longNameLine2).c_str();
            } else {
                addElement(longNameKey.c_str(), "string", "512", config.longName.c_str());
            }

            addElement(shortNameKey.c_str(), "string", "512", config.shortName.c_str());
            addElement(publisherKey.c_str(), "string", "256", "");
        }

        // Add-on unique IDs
        for (int i = 0; i < 32; ++i) {
            std::string addonKey = "add_on_unique_id" + std::to_string(i);
            addElement(addonKey.c_str(), "hexBinary", "4", "00000000");
        }

        // Convert to string
        std::ostringstream oss;
        doc.save(oss, "  ");
        return oss.str();
    }

    Result<void> saveAppXML(const std::filesystem::path &path, const AppXMLConfig &config) {
        auto xmlResult = generateAppXML(config);
        if (!xmlResult) {
            return std::unexpected(xmlResult.error());
        }

        std::ofstream file(path);
        if (!file) {
            return std::unexpected(ErrorCode::IOError);
        }

        file << *xmlResult;
        return {};
    }

    Result<void> saveMetaXML(const std::filesystem::path &path, const MetaXMLConfig &config) {
        auto xmlResult = generateMetaXML(config);
        if (!xmlResult) {
            return std::unexpected(xmlResult.error());
        }

        std::ofstream file(path);
        if (!file) {
            return std::unexpected(ErrorCode::IOError);
        }

        file << *xmlResult;
        return {};
    }

} // namespace wiivc::xmlgen
