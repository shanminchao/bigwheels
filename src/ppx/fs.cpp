// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ppx/fs.h"

#include <fstream>
#include <filesystem>
#include <vector>
#include <optional>

#if defined(PPX_ANDROID)
#   include <game-activity/native_app_glue/android_native_app_glue.h>
android_app* gAndroidContext;
#endif

namespace ppx::fs {

#if defined(PPX_ANDROID)
void set_android_context(android_app* androidContext)
{
    gAndroidContext = androidContext;
}
#endif

bool File::Exists(const char *path)
{
#if defined(PPX_ANDROID)
    AAsset* temp = AAssetManager_open(gAndroidContext->activity->assetManager,
                                      path, AASSET_MODE_BUFFER);
    if (temp != nullptr) {
        AAsset_close(temp);
        return true;
    }
    return false;
#else
    return std::filesystem::exists(path);
#endif
}

bool File::Open(const char *path)
{
#if defined(PPX_ANDROID)
    mFile = AAssetManager_open(gAndroidContext->activity->assetManager,
                               path, AASSET_MODE_BUFFER);
    return mFile != nullptr;
#else
    mStream.open(path, std::ios::binary);
#endif
}

bool File::IsOpen() const
{
#if defined(PPX_ANDROID)
    return mFile != nullptr;
#else
    return mStream.is_open();
#endif
}

size_t File::Read(void *buf, size_t count)
{
#if defined(PPX_ANDROID)
    return AAsset_read(mFile, buf, count);
#else
    mStream.read(reinterpret_cast<char*>(buf), count);
    return mStream.gcount();
#endif
}

size_t File::GetLength() const
{
#if defined(PPX_ANDROID)
    return AAsset_getLength(mFile);
#else
    int pos = mStream.tellg();
    mStream.seekg(0, std::ios::end);
    size_t size = mStream.tellg();
    mStream.seekg(pos, std::ios::beg);
    return size;
#endif
}

void File::Close()
{
#if defined(PPX_ANDROID)
    AAsset_close(mFile);
#else
    mStream.close();
#endif
}

std::optional<std::vector<char>> load_file(const std::filesystem::path& path)
{
    std::vector<char> data;

    ppx::fs::File file;
    if (!file.Open(path.c_str())) {
        return std::nullopt;
    }

    size_t size = file.GetLength();
    data.resize(size);
    file.Read(data.data(), size);
    file.Close();
    return data;
}

} // namespace ppx::fs
