/*
 * Copyright (C) 2015  Andrew Gunnerson <andrewgunnerson@gmail.com>
 *
 * This file is part of MultiBootPatcher
 *
 * MultiBootPatcher is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MultiBootPatcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MultiBootPatcher.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "wipe.h"

#include <sys/stat.h>
#include <unistd.h>

#include "multiboot.h"
#include "util/delete.h"
#include "util/logging.h"

namespace mb
{

/*!
 * \brief Log deletion of file
 *
 * \note The path will be deleted only if it is a regular file.
 *
 * \param path File to delete
 *
 * \return True if file was deleted or doesn't exist. False, otherwise.
 */
static bool log_wipe_file(const std::string &path)
{
    LOGV("Wiping file %s", path.c_str());

    struct stat sb;
    if (stat(path.c_str(), &sb) < 0) {
        if (errno != ENOENT) {
            LOGE("%s: Failed to stat: %s", path.c_str(), strerror(errno));
            return false;
        } else {
            return true;
        }
    }

    if (!S_ISREG(sb.st_mode)) {
        LOGE("%s: Cannot wipe file: not a regular file", path.c_str());
        return false;
    }

    bool ret = unlink(path.c_str()) == 0 || errno == ENOENT;
    LOGV("-> %s", ret ? "Succeeded" : "Failed");
    return ret;
}

/*!
 * \brief Log wiping of directory
 *
 * \note The path will be wiped only if it is a directory. The recursive
 *       deletion does not follow symlinks.
 *
 * \param mountpoint Mountpoint root to wipe
 * \param wipe_media Whether the first-level "media" path should be deleted
 *
 * \return True if the path was wiped or doesn't exist. False, otherwise
 */
static bool log_wipe_directory(const std::string &mountpoint, bool wipe_media)
{
    LOGV("Wiping directory %s%s", mountpoint.c_str(),
         wipe_media ? "" : " (excluding media directory)");

    struct stat sb;
    if (stat(mountpoint.c_str(), &sb) < 0) {
        if (errno != ENOENT) {
            LOGE("%s: Failed to stat: %s", mountpoint.c_str(), strerror(errno));
            return false;
        } else {
            return true;
        }
    }

    if (!S_ISDIR(sb.st_mode)) {
        LOGE("%s: Cannot wipe path: not a directory", mountpoint.c_str());
        return false;
    }

    bool ret = wipe_directory(mountpoint, wipe_media);
    LOGV("-> %s", ret ? "Succeeded" : "Failed");
    return ret;
}

static bool log_delete_recursive(const std::string &path)
{
    LOGV("Recursively deleting %s", path.c_str());
    bool ret = util::delete_recursive(path);
    LOGV("-> %s", ret ? "Succeeded" : "Failed");
    return ret;
}

bool wipe_system(const std::shared_ptr<Rom> &rom)
{
    std::string path = rom->full_system_path();
    if (path.empty()) {
        LOGE("Failed to determine full system path");
        return false;
    }

    bool ret;
    if (rom->system_is_image) {
        ret = log_wipe_file(path);
    } else {
        ret = log_wipe_directory(path, true);
        // Try removing ROM's /system if it's empty
        remove(path.c_str());
    }
    return ret;
}

bool wipe_cache(const std::shared_ptr<Rom> &rom)
{
    std::string path = rom->full_cache_path();
    if (path.empty()) {
        LOGE("Failed to determine full cache path");
        return false;
    }

    bool ret;
    if (rom->cache_is_image) {
        ret = log_wipe_file(path);
    } else {
        ret = log_wipe_directory(path, true);
        // Try removing ROM's /cache if it's empty
        remove(path.c_str());
    }
    return ret;
}

bool wipe_data(const std::shared_ptr<Rom> &rom)
{
    std::string path = rom->full_data_path();
    if (path.empty()) {
        LOGE("Failed to determine full data path");
        return false;
    }

    bool ret;
    if (rom->data_is_image) {
        ret = log_wipe_file(path);
    } else {
        ret = log_wipe_directory(path, false);
        // Try removing ROM's /data/media and /data if they're empty
        remove((path + "/media").c_str());
        remove(path.c_str());
    }
    return ret;
}

bool wipe_dalvik_cache(const std::shared_ptr<Rom> &rom)
{
    if (rom->data_is_image || rom->cache_is_image) {
        LOGE("Wiping dalvik-cache for ROMs that use data or cache images is "
             "currently not supported.");
        return false;
    }

    // Most ROMs use /data/dalvik-cache, but some use
    // /cache/dalvik-cache, like the jflte CyanogenMod builds
    std::string data_path = rom->full_data_path();
    std::string cache_path = rom->full_cache_path();

    if (data_path.empty()) {
        LOGE("Failed to determine full data path");
        return false;
    } else if (cache_path.empty()) {
        LOGE("Failed to determine full cache path");
        return false;
    }

    data_path += "/dalvik-cache";
    cache_path += "/dalvik-cache";

    // util::delete_recursive() returns true if the path does not
    // exist (ie. returns false only on errors), which is exactly
    // what we want
    return log_delete_recursive(data_path)
            && log_delete_recursive(cache_path);
}

bool wipe_multiboot(const std::shared_ptr<Rom> &rom)
{
    // Delete /data/media/0/MultiBoot/[ROM ID]
    std::string multiboot_path("/data/media/0/MultiBoot/");
    multiboot_path += rom->id;
    return log_delete_recursive(multiboot_path);
}

}
