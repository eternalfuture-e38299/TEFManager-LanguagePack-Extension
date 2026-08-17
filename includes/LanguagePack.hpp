/*******************************************************************************
 * languagepack_extension - LanguagePack
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
 * Created: 2026/8/1
 *******************************************************************************/

#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "tefkernel-cpp-wrapper/tefkernel/patchlib/type.h"

struct PackEntry {
    std::filesystem::path file;
    int priority;

    PackEntry() : priority(0) {
    }
};

#if __ANDROID__
inline const std::vector<std::string> languages_container{
    "English",
    "German",
    "Italian",
    "Portuguese",
    "Russian",
    "French",
    "Spanish"
};

inline const std::vector<std::string> languages_code{
    "en-US", // English (United States)
    "de-DE", // German (Germany)
    "it-IT", // Italian (Italy)
    "pt-PT", // Portuguese (Portugal)
    "ru-RU", // Russian (Russia)
    "fr-FR", // French (France)
    "es-ES" // Spanish (Spain)
};
#else
inline const std::vector<std::string> languages_container{
    "English",
    "German",
    "Italian",
    "French",
    "Spanish",
    "Russian",
    "Chinese",
    "Portuguese",
    "Polish",
    "Japanese",
    "Korean",
    "ChineseTraditional"
};

inline const std::vector<std::string> languages_code{
    "en-US",
    "de-DE",
    "it-IT",
    "fr-FR",
    "es-ES",
    "ru-RU",
    "zh-Hans",
    "pt-BR",
    "pl-PL",
    "ja-JP",
    "ko-KR",
    "zh-Hant"
};
#endif

inline const std::unordered_map<std::string, int> code_id = {
    {"en-US", 1}, // English
    {"de-DE", 2}, // German
    {"it-IT", 3}, // Italian
    {"fr-FR", 4}, // French
    {"es-ES", 5}, // Spanish
    {"ru-RU", 6}, // Russian
    {"zh-Hans", 7}, // Chinese (Simplified)
    {"pt-BR", 8}, // Portuguese (Brazil)
    {"pl-PL", 9}, // Polish
    {"ja-JP", 10}, // Japanese
    {"ko-KR", 11}, // Korean
    {"zh-Hant", 12} // Chinese (Traditional)
};

inline const std::unordered_map<int, std::string> id_code = {
    {1, "en-US"}, // English
    {2, "de-DE"}, // German
    {3, "it-IT"}, // Italian
    {4, "fr-FR"}, // French
    {5, "es-ES"}, // Spanish
    {6, "ru-RU"}, // Russian
    {7, "zh-Hans"}, // Chinese (Simplified)
    {8, "pt-BR"}, // Portuguese (Brazil)
    {9, "pl-PL"}, // Polish
    {10, "ja-JP"}, // Japanese
    {11, "ko-KR"}, // Korean
    {12, "zh-Hant"} // Chinese (Traditional)
};

std::string LanguageCodeToName(const std::string &code);

std::string LanguageNameToCode(const std::string &name);

int LanguageCodeToId(const std::string &code);

std::string LanguageIdToCode(int id);

enum class PackType {
    LanguagePack,
    PatchPack
};

struct PackInfo {
    std::string name;
    std::string author;
    std::string languagecode;
    PackType type;
    std::string fileextension;
    std::string displayname;
};

class LanguagePack {
    PackEntry _entry{};
    PackInfo _info{};
    std::unordered_map<std::string, std::vector<uint8_t> > _data{};
    void* _handle{};

public:
    explicit LanguagePack(PackEntry entry);

    PackInfo GetInfo() const;

    std::vector<std::string> GetFileList() const;

    std::vector<uint8_t> GetFileData(const std::string &filename) const;

    std::string GetFileContentString(const std::string &filename) const;

    void LoadText(patch_handle_t instance, const std::function<void(patch_handle_t instance, const std::string& str)>& loadTextFromStr) const;
};