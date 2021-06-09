#include "pch.h"
#include "SystemFunctions.h"

#include <stdlib.h>
#include <Windows.h>

using namespace std;
using namespace std::filesystem;

path SystemFunctions::getHomeDir() {
    char* buff;
    size_t numelem;

    _dupenv_s(&buff, &numelem, "USERPROFILE");
    path wallpaper_folder(buff);
    free(buff);

    return wallpaper_folder;
}

void SystemFunctions::messagebox(string msg) {
    wchar_t* wmsg = new wchar_t[msg.length() + 1];
    mbstowcs_s(NULL, wmsg, msg.length() + 1, msg.c_str(), _TRUNCATE);
    MessageBox(
        NULL,
        wmsg,
        L"WallpaperSwitcher Error",
        MB_OK
    );
    delete[] wmsg;
}