/*******************************************************************************
 * languagepack_extension - LanguageManager
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
 * Created: 2026/8/2
 *******************************************************************************/

#include "LanguageManager.hpp"
#include "core.hpp"
#include "Log.hpp"

#include "tefkernel-cpp-wrapper/patchlib/struct/string.hpp"


static void LoadTextFromStr(patch_handle_t instance, const std::string &str) {
    const TEFKernel::PatchLib::Struct::String string(str);
#if __ANDROID__
    LanguageManager::LoadText.InvokeVoid(instance, string.GetHandle());
#else
    LanguageManager::LoadText.InvokeVoid(instance, string.GetHandle(), true);
#endif
}

bool LanguageManager::SetLanguageHook(patch_handle_t instance, void **args,
                                      const patch_method_signature_t *sig_info, void *result) {
    // ========== 完整的参数检查 ==========
    if (!args) {
        LOGW("SetLanguageHook: args is NULL");
        return true;
    }

    if (!args[0]) {
        LOGW("SetLanguageHook: args[0] is NULL");
        return true;
    }

    // 检查 args[0] 指向的值是否为有效的 handle
    patch_handle_t culture_handle = *static_cast<patch_handle_t *>(args[0]);
    if (!culture_handle) {
        LOGW("SetLanguageHook: culture_handle is NULL (0x%p)", culture_handle);
        return true;
    }
    // =====================================

    const auto ActiveCulture = get_ActiveCulture.Invoke<patch_handle_t>(instance);
    const auto culture = culture_handle;
    const auto newLegacyId = LegacyId.GetValue<int>(culture);

    if (ActiveCulture) {
        if (const auto oldLegacyId = LegacyId.GetValue<int>(ActiveCulture); newLegacyId == oldLegacyId)
            return true;
    } else {
        LOGW("SetLanguageHook: ActiveCulture is NULL, skipping language change check");
    }

    const auto code = LanguageIdToCode(newLegacyId);
    LOGI("SetLanguageHook: Switching to language code: '%s'", code.c_str());

    auto set_language = [](patch_handle_t instance, const LanguagePack* pack) {
        const auto info = pack->GetInfo();
        LOGI("SetLanguageHook: Found language pack: '%s' (code: %s)",
             info.name.c_str(), info.languagecode.c_str());

        const auto newCulture = FromLegacyId.Invoke<patch_handle_t>(nullptr, LanguageCodeToId(info.languagecode));
        if (!newCulture) {
            LOGE("SetLanguageHook: Failed to create new culture");
        }

        LoadLanguage.InvokeVoid(instance, newCulture);
        patchlib_free(newCulture);

        if (!g_language_list_str.empty()) {
            LoadTextFromStr(instance, g_language_list_str);
        } else {
            LOGW("SetLanguageHook: g_language_list_str is empty, skipping");
        }

        ProcessCopyCommandsInTexts.InvokeVoid(instance);
    };

    if (const auto it = g_language_indexes.find(code); it != g_language_indexes.end()) {
        set_language(instance, it->second);
        return true;
    }

    LOGW("SetLanguageHook: Language pack not found for code '%s' use default language pack", code.c_str());

    set_language(instance, &language_packs[0]);

    return true;
}

void LanguageManager::LoadLanguageHook(patch_handle_t instance, void **args, void *result,
                                       const patch_method_signature_t *sig_info) {
    // ========== 完整的参数检查 ==========
    if (!args) {
        LOGW("LoadLanguageHook: args is NULL");
        return;
    }

    if (!args[0]) {
        LOGW("LoadLanguageHook: args[0] is NULL");
        return;
    }

    patch_handle_t culture_handle = *static_cast<patch_handle_t *>(args[0]);
    if (!culture_handle) {
        LOGW("LoadLanguageHook: culture_handle is NULL (0x%p)", culture_handle);
        return;
    }
    // =====================================

    const auto culture = culture_handle;
    const auto newLegacyId = LegacyId.GetValue<int>(culture);
    const auto code = LanguageIdToCode(newLegacyId);

    LOGI("LoadLanguageHook: Loading language: '%s' (ID: %d)", code.c_str(), newLegacyId);

    // 加载主语言包
    bool found_main_pack = false;
    for (int i = 0; i < language_packs.size(); ++i) {
        if (const auto &pack = language_packs[i]; pack.GetInfo().languagecode == code) {
            LOGI("LoadLanguageHook: Found matching main pack: '%s'", pack.GetInfo().name.c_str());
            found_main_pack = true;

            for (const auto &file_list = pack.GetFileList(); const auto &filename : file_list) {
                if (filename.starts_with("localization/")) {
                    LOGI("LoadLanguageHook: Loading main file: '%s' (%zu bytes)",
                         filename.c_str(), pack.GetFileContentString(filename).size());
                    LoadTextFromStr(instance, pack.GetFileContentString(filename));
                }
            }

#if __ANDROID__
            const auto newCulture = FromLegacyId.Invoke<patch_handle_t>(nullptr, LanguageCodeToId(languages_code[i]));
            set_ActiveCulture.InvokeVoid(instance, newCulture);
#endif

            break;
        }
    }

    if (!found_main_pack) {
        LOGW("LoadLanguageHook: No main language pack found for code '%s'", code.c_str());
    }

    // 加载补丁包
    if (const auto it = g_language_patch_indexes.find(code); it != g_language_patch_indexes.end()) {
        LOGI("LoadLanguageHook: Found %zu patch packs for code '%s'", it->second.size(), code.c_str());
        for (const auto pack: it->second) {
            LOGI("LoadLanguageHook: Processing patch pack: '%s'", pack->GetInfo().name.c_str());
            for (const auto &file_list = pack->GetFileList(); const auto &filename: file_list) {
                if (filename.starts_with("localization/")) {
                    LOGI("LoadLanguageHook: Loading patch file: '%s' (%zu bytes)",
                         filename.c_str(), pack->GetFileContentString(filename).size());
                    LoadTextFromStr(instance, pack->GetFileContentString(filename));
                }
            }
        }
    } else {
        LOGI("LoadLanguageHook: No patch packs found for code '%s'", code.c_str());
    }

    if (!g_language_list_str.empty() && !language_packs.empty()) {
        LOGI("LoadLanguageHook: Loading language list (%zu bytes)", g_language_list_str.size());
        LoadTextFromStr(instance, g_language_list_str);
    } else {
        LOGW("LoadLanguageHook: g_language_list_str is empty, skipping");
    }

    ProcessCopyCommandsInTexts.InvokeVoid(instance);
}