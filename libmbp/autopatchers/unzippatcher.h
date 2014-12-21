/*
 * Copyright (C) 2014  Xiao-Long Chen <chenxiaolong@cxl.epac.to>
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

#pragma once

#include <memory>

#include "patcherconfig.h"
#include "patcherinterface.h"


class UnzipPatcher : public AutoPatcher
{
public:
    explicit UnzipPatcher(const PatcherConfig * const pc,
                          const FileInfo * const info,
                          const PatchInfo::AutoPatcherArgs &args);
    ~UnzipPatcher();

    static const std::string Id;

    virtual PatcherError error() const override;

    virtual std::string id() const override;

    virtual std::vector<std::string> newFiles() const override;
    virtual std::vector<std::string> existingFiles() const override;

    virtual bool patchFiles(const std::string &directory,
                            const std::vector<std::string> &bootImages) override;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
