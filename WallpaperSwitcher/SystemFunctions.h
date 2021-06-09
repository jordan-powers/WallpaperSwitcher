#pragma once

#include <filesystem>
#include <string>

namespace SystemFunctions {
    std::filesystem::path getHomeDir();
    void messagebox(std::string);

    void setWallpaper(std::filesystem::path);
    void setLockScreen(std::filesystem::path);
}