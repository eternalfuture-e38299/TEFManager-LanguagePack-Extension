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

#pragma once

#include "tefkernel-cpp-wrapper/patchlib/method.hpp"
#include "tefkernel-cpp-wrapper/patchlib/field.hpp"

namespace LanguageManager {
    // GameCulture
    inline TEFKernel::PatchLib::Field LegacyId;
    inline TEFKernel::PatchLib::Method FromLegacyId; // static GameCulture FromLegacyId(int id)

    // LanguageManager
    inline TEFKernel::PatchLib::Method ProcessCopyCommandsInTexts; // void ProcessCopyCommandsInTexts()
    inline TEFKernel::PatchLib::Method LoadLanguage;
    // void LoadLanguage(GameCulture culture) android will add a bool arg
    inline TEFKernel::PatchLib::Method get_ActiveCulture; // GameCulture get_ActiveCulture()
    inline TEFKernel::PatchLib::Method set_ActiveCulture; // void set_ActiveCulture(GameCulture value)

    // android -> void LoadLanguageFromFileText(string fileText)
    // desktop -> void LoadLanguageFromFileTextJson(string fileText, bool canCreateCategories)
    inline TEFKernel::PatchLib::Method LoadText;


    bool SetLanguageHook(patch_handle_t instance, void **args,
                         const patch_method_signature_t *sig_info, void *result);

    void LoadLanguageHook(patch_handle_t instance, void **args, void *result,
                          const patch_method_signature_t *sig_info);
}
