#include "pch.h"
#include "App.h"

#include "SystemInterface.h"

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
    : system(SystemInterface::getInstance())
    , m_source(system)
{}

inline double time_now() {
    chrono::microseconds ms = chrono::duration_cast<chrono::microseconds>(
        chrono::system_clock::now().time_since_epoch()
    );
    return ms.count() / (double)1000000;
}

optional<vector<filesystem::path>> App::get_next_wallpapers() {
    auto wallpapers = m_source.getWallpapers();
    if (wallpapers.size() == 0) {
        SystemInterface::messagebox("No wallpapers found!");
        return {};
    }
    sort(wallpapers.begin(), wallpapers.end());

    filesystem::path *current_wallpaper;
    if (m_current_wallpaper.has_value()) {
        current_wallpaper = &m_current_wallpaper.value();
    }
    else {
        srand((unsigned)(time_now() * 1000));
        int next_idx = wallpapers.size() * (static_cast<float>(rand()) / RAND_MAX);
        current_wallpaper = &(wallpapers[next_idx]);
    }

    auto curr_wallpaper_iter = std::find(wallpapers.begin(), wallpapers.end(), *current_wallpaper);
    vector<filesystem::path>::iterator next_wallpaper_iter;
    if (curr_wallpaper_iter != wallpapers.end()) {
        next_wallpaper_iter = curr_wallpaper_iter + 1;
    }
    else {
        next_wallpaper_iter = wallpapers.begin();
        while (next_wallpaper_iter != wallpapers.end() && *next_wallpaper_iter < *current_wallpaper) {
            next_wallpaper_iter += 1;
        }
    }

    if (next_wallpaper_iter == wallpapers.end()) {
        next_wallpaper_iter = wallpapers.begin();
    }

    assert(next_wallpaper_iter != wallpapers.end());

    m_current_wallpaper = *next_wallpaper_iter;

    vector<filesystem::path> next_wallpapers;
    int monitor_count = system->getMonitorCount();
    for (int i = 0; i < monitor_count; ++i) {
        next_wallpapers.push_back(*next_wallpaper_iter);
        next_wallpaper_iter += 1;
        if (next_wallpaper_iter == wallpapers.end()) {
            next_wallpaper_iter = wallpapers.begin();
        }
    }

    return next_wallpapers;
}

void App::update_wallpaper() {
    auto next_wallpapers = get_next_wallpapers();
    if (next_wallpapers.has_value()) {
        system->setWallpapers(next_wallpapers.value());
        system->setLockScreen(next_wallpapers.value()[0]);
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