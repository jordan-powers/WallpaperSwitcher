#include "pch.h"
#include "App.h"

#include "SystemFunctions.h"

#include <vector>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <thread>

static constexpr int sleep_time = 120;

using namespace std;

App App::m_instance;

App::App()
    : m_source()
    , m_database(m_source.getDatabase().string())
{
    
}

inline double time_now() {
    chrono::microseconds ms = chrono::duration_cast<chrono::microseconds>(
        chrono::system_clock::now().time_since_epoch()
    );
    return ms.count() / (double)1000000;
}

void App::update_db() {
    auto entries = m_database.getAllEntries();
    auto wallpapers = m_source.getWallpapers();

    vector<string> dbSavedWallpapers;
    for (auto& entry : entries) {
        dbSavedWallpapers.push_back(entry.path);
    }
    vector<string> foundWallpapers;
    for (auto& wallpaper : wallpapers) {
        foundWallpapers.push_back(wallpaper.string());
    }

    sort(foundWallpapers.begin(), foundWallpapers.end());

    vector<string> deleteFromDb;
    vector<string> addToDb;

    set_difference(dbSavedWallpapers.begin(), dbSavedWallpapers.end(), foundWallpapers.begin(), foundWallpapers.end(), back_insert_iterator(deleteFromDb));
    set_difference(foundWallpapers.begin(), foundWallpapers.end(), dbSavedWallpapers.begin(), dbSavedWallpapers.end(), back_insert_iterator(addToDb));

    for (auto& item : deleteFromDb) {
        m_database.deleteEntry(item);
    }
    for (auto& item : addToDb) {
        m_database.addEntry({ item, 0 });
    }
}

void App::update_outputs() {
    auto nextEntryOptional = m_database.getNextEntry();

    if (!nextEntryOptional.has_value()) {
        SystemFunctions::messagebox("No wallpapers found!");
        return;
    }

    auto entry = nextEntryOptional.value();

    Database::Entry updatedEntry(entry.path, time_now());

    m_database.saveEntry(updatedEntry);

    filesystem::path file(entry.path);

    SystemFunctions::setWallpaper(file);
    SystemFunctions::setLockScreen(file);
}

void App::show_next() {
    update_db();
    update_outputs();
}

void App::main() {
    m_is_exited = false;
    while (!m_is_exited) {
        show_next();
        this_thread::sleep_for(chrono::milliseconds(sleep_time * 1000));
    }
}