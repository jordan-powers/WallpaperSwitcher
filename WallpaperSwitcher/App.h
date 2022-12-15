#pragma once

#include "FileSource.h"

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

    std::optional<std::filesystem::path> get_next_wallpaper();
    void update_wallpaper();
};