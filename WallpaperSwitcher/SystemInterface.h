#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <shobjidl_core.h>

class SystemInterface {
public:
    static SystemInterface* getInstance();
    static void messagebox(const std::string&);
    std::filesystem::path getHomeDir();

    int getMonitorCount();
    void setWallpapers(std::vector<std::filesystem::path>);
    void setLockScreen(std::filesystem::path);
private:
    SystemInterface();
    ~SystemInterface();

    std::vector<LPWSTR> monitor_uids;
    void update_monitor_uids(int monitor_count);


    IDesktopWallpaper* pDesktopWallpaper{ nullptr };
    static SystemInterface* instance;
};