#pragma once

#include "FileSource.h"
#include "SystemInterface.h"

#include <optional>

class App
{
public:
    static App* getInstance() { return &m_instance;  }

    void show_next();
    void main();

    void interrupt() { m_is_exited = true; }

private:
    static App m_instance;
    bool m_is_exited = false;

    std::optional<std::filesystem::path> m_current_wallpaper;
    FileSource m_source;

    App();

    std::optional<std::vector<std::filesystem::path>> get_next_wallpapers();
    void update_wallpaper();

    SystemInterface* system;
};