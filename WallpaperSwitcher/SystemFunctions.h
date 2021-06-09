#pragma once

#include <filesystem>
#include <string>

namespace SystemFunctions {
    std::filesystem::path getHomeDir();
    void messagebox(std::string);
}