#pragma once
#include <vector>
#include <set>
#include <filesystem>

#include "SystemInterface.h"

class FileSource {
public:
    FileSource(SystemInterface* system);

    virtual ~FileSource();
    const std::vector<std::filesystem::path> getWallpapers() const;

private:
    std::filesystem::path wallpaperPath{};
    const std::set<std::string> allowedExtensions{ ".jpg", ".png" };
};

