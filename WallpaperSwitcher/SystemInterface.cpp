#include "pch.h"
#include "SystemInterface.h"

#include <stdlib.h>
#include <Windows.h>

#include <locale>
#include <codecvt>
#include <string>

#include <pplawait.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.System.UserProfile.h>

using namespace std;
using namespace std::filesystem;

using namespace winrt;
using namespace Windows::Storage;
using namespace Windows::System::UserProfile;

SystemInterface* SystemInterface::instance = nullptr;

inline wstring to_wstring(const string& str) {
    int wchars_num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t *wstr = new wchar_t[wchars_num];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, wchars_num);
    wstring out(wstr);
    delete[] wstr;
    return out;
}

inline void check(HRESULT hr, const string& method) {
    if (hr != S_OK) {
        SystemInterface::messagebox("ERROR: failed winapi call - " + method);
        exit(1);
    }
}

SystemInterface* SystemInterface::getInstance() {
    if (!instance) {
        instance = new SystemInterface();
    }

    return instance;
}

SystemInterface::SystemInterface() {
    HRESULT hr = CoInitialize(nullptr);
    check(hr, "CoInitialize()");

    hr = CoCreateInstance(__uuidof(DesktopWallpaper), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pDesktopWallpaper));
    check(hr, "CoCreateInstance()");
}

SystemInterface::~SystemInterface() {
    if (pDesktopWallpaper) {
        pDesktopWallpaper->Release();
    }

    CoUninitialize();
}

path SystemInterface::getHomeDir() {
    char* buff;
    size_t numelem;

    _dupenv_s(&buff, &numelem, "USERPROFILE");
    path wallpaper_folder(buff);
    free(buff);

    return wallpaper_folder;
}

void SystemInterface::messagebox(const string& msg) {
    wstring wmsg = to_wstring(msg);
    MessageBox(
        NULL,
        wmsg.c_str(),
        L"WallpaperSwitcher Error",
        MB_OK
    );
}

void SystemInterface::update_monitor_uids(int monitor_count) {
    HRESULT hr;

    LPWSTR curr_uid;
    for (int i = 0; i < monitor_count; ++i) {
        hr = pDesktopWallpaper->GetMonitorDevicePathAt(i, &curr_uid);
        check(hr, "GetMonitorDevicePathAt()");

        monitor_uids.push_back(curr_uid);
    }
}

int SystemInterface::getMonitorCount() {
    UINT count;
    HRESULT hr;

    hr = pDesktopWallpaper->GetMonitorDevicePathCount(&count);
    check(hr, "GetMonitorDevicePathCount()");

    if (count != monitor_uids.size()) {
        update_monitor_uids(count);
    }

    return count;
}

void SystemInterface::setWallpapers(vector<path> wallpapers) {
    HRESULT hr;
    
    assert(wallpapers.size() == monitor_uids.size());

    for (int i = 0; i < wallpapers.size(); ++i) {
        wstring path = to_wstring(wallpapers[i].string());
        hr = pDesktopWallpaper->SetWallpaper(monitor_uids[i], path.c_str());
    }
}

void SystemInterface::setLockScreen(path lockscreen) {
    wstring lstring = to_wstring(lockscreen.string());

    concurrency::create_task([&] {
        StorageFile file = StorageFile::GetFileFromPathAsync(lstring.c_str()).get();
        LockScreen::SetImageFileAsync(file).get();
    }).get();
}
