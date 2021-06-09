#include "pch.h"
#include "SystemFunctions.h"

#include <stdlib.h>
#include <Windows.h>

#include <winrt/base.h>
#include <winrt/Windows.System.UserProfile.h>
#include <winrt/Windows.Storage.h>

using namespace std;
using namespace std::filesystem;

using namespace winrt;
using namespace Windows::Storage;
using namespace Windows::System::UserProfile;

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

void SystemFunctions::setWallpaper(path wallpaper) {
    SystemParametersInfoA(20, 0, (PVOID)wallpaper.string().c_str(), 0);
}

void SystemFunctions::setLockScreen(path lockscreen) {
    std::string lstring = lockscreen.string();
    wchar_t* wlstring = new wchar_t[lstring.length() + 1];
    mbstowcs_s(NULL, wlstring, lstring.length() + 1, lstring.c_str(), _TRUNCATE);

    StorageFile file = StorageFile::GetFileFromPathAsync(wlstring).get();
    LockScreen::SetImageFileAsync(file).get();

    delete[] wlstring;
}