#include "pch.h"
#include "FileSource.h"
#include "SystemFunctions.h"

using namespace std;
using namespace std::filesystem;

FileSource::FileSource() {
    wallpaperPath = SystemFunctions::getHomeDir() / "Documents" / "wallpapers";
}

FileSource::~FileSource() {}

const vector<path> FileSource::getWallpapers() const {
    vector<path> paths;
    for (auto& p : recursive_directory_iterator(wallpaperPath)) {
        path item = p.path();

        if (!p.is_regular_file())
            continue;
        if (!item.has_extension())
            continue;
        if (allowedExtensions.find(item.extension().string()) == allowedExtensions.end())
            continue;
        paths.push_back(item);
    }
    return paths;
}
