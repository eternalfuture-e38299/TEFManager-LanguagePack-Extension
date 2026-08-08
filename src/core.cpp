/*******************************************************************************
 * languagepack_extension - core
 * Copyright (C) 2026 eternalfuture-e38299
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Author: eternalfuture-e38299
 * GitHub: https://github.com/eternalfuture-e38299
 * Created: 2026/6/27
 *******************************************************************************/

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

#include "tefkernel-cpp-wrapper/tefkernel/module/module_core.h"
#include "tefkernel-cpp-wrapper/patchlib/property.hpp"
#include "lib/json.hpp"
#include "LanguagePack.hpp"
#include "Log.hpp"
#include "core.hpp"
#include "LanguageManager.hpp"

static constexpr module_info_t g_module_info = {
    .pkg_id = "eternal.future.languagepackextension", // 唯一包名
    .name = "LanguagePack Extension", // 插件名称
    .author = "eternalfuture-e38299", // 作者
    .version = "1.0.0", // 版本
    .version_code = 1, // 版本代码
    .api_version = 1, // API版本
    .plugin_dependencies_sizes = 0, // 依赖数量（无依赖设为0）
    .plugin_dependencies = nullptr, // 依赖列表（无依赖设为NULL）
};

static bool load_json(const std::filesystem::path &path, const std::filesystem::path &dir, std::vector<PackEntry> &output) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            LOGW("Warning: Cannot open %s\n", path.c_str());
            return false;
        }

        nlohmann::json j;
        file >> j;

        if (!j.is_array()) {
            LOGE("Error: JSON root must be an array\n");
            return false;
        }

        // 临时存储所有 enable=true 的条目
        std::vector<PackEntry> temp;

        for (const auto &item: j) {
            if (item.value("enable", false)) {
                PackEntry entry;
                entry.file = dir / item.value("file", "");
                entry.priority = item.value("priority", 0);
                temp.push_back(entry);
            }
        }

        // 按优先级排序
        std::ranges::sort(temp,
                          [](const PackEntry &a, const PackEntry &b) {
                              return a.priority < b.priority;
                          });

        output = std::move(temp); // 移动赋值，高效
        return true;
    } catch (const nlohmann::json::parse_error &e) {
        LOGE("JSON parse error in %s: %s", path.c_str(), e.what());
        return false;
    } catch (const std::exception &e) {
        LOGE("Error reading %s: %s", path.c_str(), e.what());
        return false;
    }
}

static std::string MakeLanguagesList(const std::unordered_map<std::string, std::string> &list) {
    nlohmann::json j;
    nlohmann::json languageJson = nlohmann::json::object();

    for (const auto& lang: languages_container) {
        languageJson[lang] = "";
    }

    // 先将list中的所有键值对添加到languageJson中
    for (const auto& [key, value] : list) {
        languageJson[key] = value;
    }

    // 将languageJson赋值给j的"Language"字段
    j["Language"] = languageJson;

    return j.dump();
}

static bool init_module(module_entry_t *entry) {
    (void) entry; // 消除未使用参数警告

    std::vector<PackEntry> languagePackEntries{};
    std::vector<PackEntry> patchPackEntries{};

    const std::filesystem::path private_dir(entry->private_dir);
    const auto configPath = private_dir / "config.json";
    const auto patchConfigPath = private_dir / "language_patch_packs.json";

    load_json(configPath, private_dir / "language_packs", languagePackEntries);
    load_json(patchConfigPath, private_dir / "language_patch_packs", patchPackEntries);

    language_packs.reserve(languagePackEntries.size());
    language_patch_packs.reserve(patchPackEntries.size());

    for (int i = 0; i < languagePackEntries.size(); ++i) {
        const auto &code = languages_code[i];
        auto language_pack_entry = LanguagePack(languagePackEntries[i]);
        language_packs.push_back(language_pack_entry);
        g_language_indexes[code] = &language_packs.back();
    }

    std::unordered_map<std::string, std::string> language_list{};
    LOGI("Initialize language list");
    for (const auto& [code, pack]: g_language_indexes) {
        auto languageName = pack->GetInfo().name + " by " + pack->GetInfo().author;
        language_list[LanguageCodeToName(code)] = languageName;
        LOGI("Put: %s, into: %s", languageName.c_str(), LanguageCodeToName(code).c_str());
    }
    g_language_list_str = MakeLanguagesList(language_list);

    for (const auto &patch_entry: patchPackEntries) {
        auto patch_pack_entry = LanguagePack(patch_entry);
        const auto &code = patch_pack_entry.GetInfo().languagecode;
        language_patch_packs.emplace_back(patch_pack_entry);
        g_language_patch_indexes[code].push_back(&language_patch_packs.back());
    }

    TEFKernel::PatchLib::Type GameCulture("Terraria.Localization", "GameCulture");
    LanguageManager::LegacyId = GameCulture.GetField("LegacyId");
    LanguageManager::FromLegacyId = GameCulture.GetMethod("FromLegacyId", 1);

    TEFKernel::PatchLib::Type LanguageManager("Terraria.Localization", "LanguageManager");
    LanguageManager::ProcessCopyCommandsInTexts = LanguageManager.GetMethod("ProcessCopyCommandsInTexts", 0);

#if __ANDROID__
    LanguageManager::LoadLanguage = LanguageManager.GetMethod("LoadLanguage", 1);
#else
    LanguageManager::LoadLanguage = LanguageManager.GetMethod("ReloadLanguage", 1);
#endif

    auto ActiveCulture = LanguageManager.GetProperty("ActiveCulture");
    LanguageManager::get_ActiveCulture = ActiveCulture.GetGetMethod();
    LanguageManager::set_ActiveCulture = ActiveCulture.GetSetMethod();


#if __ANDROID__
    LanguageManager::LoadText = LanguageManager.GetMethod("LoadLanguageFromFileText", 1);
#else
    LanguageManager::LoadText = LanguageManager.GetMethod("LoadLanguageFromFileTextJson", 2);
#endif
    
    if (!language_packs.empty()) {
        auto SetLanguage = LanguageManager.GetMethod("SetLanguage", {"culture"});
        patchlib_install_prepost_hook(SetLanguage.GetHandle(), LanguageManager::SetLanguageHook, nullptr);
    }

    patchlib_install_prepost_hook(LanguageManager::LoadLanguage.GetHandle(), nullptr,
                                  LanguageManager::LoadLanguageHook);

    return true;
}

static bool cleanup_module(module_entry_t *entry) {
    (void) entry; // 消除未使用参数警告

    return true; // 返回true表示清理成功
}

static void hot_reload(module_entry_t *entry) {
    (void) entry; // 消除未使用参数警告
}

static const module_info_t *get_info() {
    return &g_module_info;
}

static constexpr module_ops_t g_module_ops = {
    .init_module = init_module,
    .cleanup_module = cleanup_module,
    .hot_reload = hot_reload,
    .get_info = get_info,
};

API_EXPORT const module_ops_t * API_CALL module_create(void) {
    return &g_module_ops;
}