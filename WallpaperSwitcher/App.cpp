#include "pch.h"
#include "App.h"

#include "SystemFunctions.h"

#include <vector>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <thread>

#include <cassert>
#include <cstdlib>

#include <iostream>

static constexpr int sleep_time = 120;

using namespace std;

App App::m_instance;

App::App()
    : m_source()
{}

inline double time_now() {
    chrono::microseconds ms = chrono::duration_cast<chrono::microseconds>(
        chrono::system_clock::now().time_since_epoch()
    );
    return ms.count() / (double)1000000;
}

optional<filesystem::path> App::get_next_wallpaper() {
    auto wallpapers = m_source.getWallpapers();
    if (wallpapers.size() == 0) {
        SystemFunctions::messagebox("No wallpapers found!");
        return {};
    }
    sort(wallpapers.begin(), wallpapers.end());

    filesystem::path *current_wallpaper;
    if (m_current_wallpaper.has_value()) {
        current_wallpaper = &(*m_current_wallpaper);
    }
    else {
        srand((unsigned)time_now());
        int next_idx = wallpapers.size() * (static_cast<float>(rand()) / RAND_MAX);
        cout << next_idx << endl;
        current_wallpaper = &(wallpapers[next_idx]);
    }

    auto curr_wallpaper_iter = std::find(wallpapers.begin(), wallpapers.end(), *current_wallpaper);
    if (curr_wallpaper_iter == wallpapers.end()) {
        wallpapers.push_back(*current_wallpaper);
        sort(wallpapers.begin(), wallpapers.end());
        curr_wallpaper_iter = std::find(wallpapers.begin(), wallpapers.end(), *current_wallpaper);
    }

    assert(curr_wallpaper_iter != wallpapers.end());

    auto next_wallpaper_iter = curr_wallpaper_iter + 1;
    if (next_wallpaper_iter == wallpapers.end()) {
        next_wallpaper_iter = wallpapers.begin();
    }

    assert(next_wallpaper_iter != wallpapers.end());

    m_current_wallpaper = *next_wallpaper_iter;

    return *next_wallpaper_iter;
}

void App::update_wallpaper() {
    auto next_wallpaper = get_next_wallpaper();
    if (next_wallpaper.has_value()) {
        SystemFunctions::setWallpaper(*next_wallpaper);
        SystemFunctions::setLockScreen(*next_wallpaper);
    }
}

void App::show_next() {
    update_wallpaper();
}

void App::main() {
    m_is_exited = false;
    while (!m_is_exited) {
        show_next();
        this_thread::sleep_for(chrono::milliseconds(sleep_time * 1000));
    }
}