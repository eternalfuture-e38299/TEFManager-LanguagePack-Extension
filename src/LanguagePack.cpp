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

#include "LanguagePack.hpp"
#include "lib/miniz.h"
#include "lib/json.hpp"

#include <stdexcept>

#include "Log.hpp"

std::string LanguageCodeToName(const std::string &code) {
    for (size_t i = 0; i < languages_code.size(); ++i) {
        if (languages_code[i] == code) {
            return languages_container[i]; // 直接返回对应位置的name
        }
    }
    throw std::invalid_argument("Unknown language code:" + code);
}

std::string LanguageNameToCode(const std::string &name) {
    for (size_t i = 0; i < languages_container.size(); ++i) {
        if (languages_container[i] == name) {
            return languages_code[i]; // 直接返回对应位置的code
        }
    }
    throw std::invalid_argument("Unknown language name: " + name);
}

int LanguageCodeToId(const std::string &code) {
    if (const auto it = code_id.find(code); it != code_id.end()) {
        return it->second;
    }
    throw std::invalid_argument("Unknown language code: " + code);
}

std::string LanguageIdToCode(const int id) {
    if (const auto it = id_code.find(id); it != id_code.end()) {
        return it->second;
    }
    throw std::invalid_argument("Unknown language id: " + std::to_string(id));
}

LanguagePack::LanguagePack(PackEntry entry) : _entry(std::move(entry)) {
    mz_zip_archive zip_archive = {};

    if (!mz_zip_reader_init_file(&zip_archive, _entry.file.string().c_str(), 0)) {
        LOGE("Failed to open zip file: %s", _entry.file.string().c_str());
        throw std::runtime_error("Failed to open language pack: " + _entry.file.string());
    }

    auto file_count = mz_zip_reader_get_num_files(&zip_archive);

    size_t total_loaded = 0;
    size_t total_size = 0;

    for (int i = 0; i < file_count; i++) {
        mz_zip_archive_file_stat file_stat;

        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
            LOGW("Unable to get information for file %d in the ZIP", i);
            continue;
        }

        if (mz_zip_reader_is_file_a_directory(&zip_archive, i))
            continue;

        std::string filename(file_stat.m_filename);

        size_t uncomp_size;
        void *p = mz_zip_reader_extract_file_to_heap(&zip_archive, file_stat.m_filename, &uncomp_size, 0);
        if (!p) {
            LOGE("Failed to extract file: %s", filename.c_str());
            continue;
        }

        _data[filename] = std::vector(static_cast<uint8_t *>(p), static_cast<uint8_t *>(p) + uncomp_size);

        total_loaded++;
        total_size += uncomp_size;

        mz_free(p);
    }

    mz_zip_reader_end(&zip_archive);

    auto config = nlohmann::json::parse(GetFileContentString("config.json"));

    _info.name = config["name"].get<std::string>();
    _info.languagecode = config.value("languagecode", "en-US");
    _info.fileextension = config.value("fileextension", "json");
    _info.author = config["author"].get<std::string>();

    auto typeStr = config.value("type", "languagepatchpack");
    std::ranges::transform(typeStr, typeStr.begin(),
                           [](unsigned char c) { return std::tolower(c); });
    _info.type = typeStr == "languagepack" ? PackType::LanguagePack : PackType::PatchPack;
    _info.displayname = config.value("displayname", _info.name + " by " + _info.author);
}

PackInfo LanguagePack::GetInfo() const {
    return _info;
}

std::vector<std::string> LanguagePack::GetFileList() const {
    std::vector<std::string> result;
    result.reserve(_data.size()); // 预分配内存，避免重新分配
    for (const auto &key: _data | std::views::keys)
        result.push_back(key);
    return result;
}

std::vector<uint8_t> LanguagePack::GetFileData(const std::string &filename) const {
    if (const auto it = _data.find(filename); it != _data.end())
        return it->second;
    return {};
}

std::string LanguagePack::GetFileContentString(const std::string &filename) const {
    const auto data = GetFileData(filename);
    std::string result(data.begin(), data.end());
    return result;
}
