#include "pch.h"
#include "SystemFunctions.h"

#include <stdlib.h>

using namespace std::filesystem;

path SystemFunctions::getHomeDir() {
    char* buff;
    size_t numelem;

    _dupenv_s(&buff, &numelem, "USERPROFILE");
    path wallpaper_folder(buff);
    free(buff);

    return wallpaper_folder;
}