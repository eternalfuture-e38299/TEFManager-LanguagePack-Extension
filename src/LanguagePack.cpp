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
#include "tefkernel-cpp-wrapper/tefkernel/tefplugin/tpf_core.h"
#include "tefkernel-cpp-wrapper/tefkernel/memdl/memdl.h"


#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_NAME "windows"
    #define DYLIB_EXT ".dll"
    #define DYLIB_PREFIX ""
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
        #define PLATFORM_NAME "ios"
    #else
        #define PLATFORM_NAME "mac"
    #endif
    #define DYLIB_EXT ".dylib"
    #define DYLIB_PREFIX "lib"
#elif defined(__ANDROID__)
    #define PLATFORM_NAME "android"
    #define DYLIB_EXT ".so"
    #define DYLIB_PREFIX "lib"
#elif defined(__linux__)
    #define PLATFORM_NAME "linux"
    #define DYLIB_EXT ".so"
    #define DYLIB_PREFIX "lib"
#else
    #define PLATFORM_NAME "unknown"
    #define DYLIB_EXT ""
    #define DYLIB_PREFIX ""
#endif

#if defined(__x86_64__) || defined(_M_X64)
    #define ARCH_NAME "x64"
#elif defined(__i386__) || defined(_M_IX86)
    #define ARCH_NAME "x86"
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ARCH_NAME "arm64"
#elif defined(__arm__) || defined(_M_ARM)
    #define ARCH_NAME "arm"
#else
    #define ARCH_NAME "unknown"
#endif

// 构建库路径宏
#define LOCALIZATION_LIB_PATH \
    "lib/" DYLIB_PREFIX "localization." PLATFORM_NAME "." ARCH_NAME DYLIB_EXT

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

    LOGI("Loaded %zu files from language pack: %s", total_loaded, _entry.file.string().c_str());

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

    LOGI("Language pack info: name='%s', language='%s', extension='%s', type='%s'",
         _info.name.c_str(), _info.languagecode.c_str(),
         _info.fileextension.c_str(), typeStr.c_str());

    // ============ 处理 localizationbinary 格式 ============
    if (_info.fileextension == "localizationbinary") {
        LOGI("Loading localizationbinary...");

        std::string lib_path = LOCALIZATION_LIB_PATH;
        auto lib_data = GetFileData(lib_path);
        if (lib_data.empty()) {
            LOGE("Failed to load library: %s", lib_path.c_str());
            _handle = nullptr;
        } else {
            _handle = memdl_open(lib_data.data(), lib_data.size(), MEMDL_LOCAL | MEMDL_LAZY);
            if (!_handle) {
                LOGE("Failed to open memory library");
            } else {
                tpf_register_shared_plugin_library(_handle);

                auto localization_data = GetFileData("localization");
                if (localization_data.empty()) {
                    LOGE("Failed to load localization data");
                } else {
                    void* sym = memdl_sym(_handle, "init");
                    if (!sym) {
                        LOGE("Failed to find 'init' symbol");
                    } else {
                        using InitFunc = void (*)(const void*, size_t);
                        auto* init = reinterpret_cast<InitFunc>(sym);
                        if (init) {
                            init(localization_data.data(), localization_data.size());
                            LOGI("Init completed");
                        } else {
                            LOGE("Failed to cast init function");
                        }
                    }
                }
            }
        }

        LOGI("localizationbinary %s ", _handle ? "loaded" : "failed");
    } else {
        LOGI("Using standard JSON format: %s", _info.fileextension.c_str());
    }
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

void LanguagePack::LoadText(patch_handle_t instance,
    const std::function<void(patch_handle_t instance, const std::string &str)>& loadTextFromStr) const {

    LOGI("LoadText called, handle: %p, type: %s", _handle, _info.fileextension.c_str());

    if (!_handle) {
        // 标准 JSON 加载
        LOGI("Loading standard JSON format");
        const auto file_list = GetFileList();
        LOGI("Found %zu files in pack", file_list.size());

        for (const auto& filename : file_list) {
            if (filename.starts_with("localization/")) {
                LOGI("Loading main file: '%s' (%zu bytes)",
                     filename.c_str(), GetFileContentString(filename).size());
                loadTextFromStr(instance, GetFileContentString(filename));
            }
        }
        LOGI("JSON loading complete");
    } else {
        // localizationbinary 加载
        LOGI("Loading localizationbinary format");
        LOGI("Getting 'load' symbol...");

        void* sym = memdl_sym(_handle, "load");
        if (!sym) {
            LOGE("Failed to find 'load' symbol");
            return;
        }

        LOGI("'load' symbol found at: %p", sym);

        using LoadFunc = void (*)(void*);
        auto* load = reinterpret_cast<LoadFunc>(sym);
        if (!load) {
            LOGE("Failed to cast 'load' function pointer");
            return;
        }

        LOGI("Calling load function with instance: %p", instance);
        load(instance);
        LOGI("Load function called successfully");
    }
}