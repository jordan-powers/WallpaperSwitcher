#include "pch.h"
#include <Windows.h>

#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <csignal>
#include <algorithm>
#include <iterator>

#include "SystemFunctions.h"
#include "FileSource.h"
#include "Database.h"

using namespace std;
using namespace std::filesystem;
using namespace std::chrono;

bool is_exited = false;


void signal_handler(int signum) {
    is_exited = true;
}

double time_now() {
    microseconds ms = duration_cast<microseconds>(
        system_clock::now().time_since_epoch()
    );
    return ms.count() / (double)1000000;
}

void update_db(Database database, const FileSource source) {
    auto entries = database.getAllEntries();
    auto wallpapers = source.getWallpapers();

    vector<string> dbSavedWallpapers;
    for (auto& entry : entries) {
        dbSavedWallpapers.push_back(entry.path);
    }
    vector<string> foundWallpapers;
    for (auto& wallpaper : wallpapers) {
        foundWallpapers.push_back(wallpaper.string());
    }

    sort(dbSavedWallpapers.begin(), dbSavedWallpapers.end());
    sort(foundWallpapers.begin(), foundWallpapers.end());

    vector<string> deleteFromDb;
    vector<string> addToDb;

    set_difference(dbSavedWallpapers.begin(), dbSavedWallpapers.end(), foundWallpapers.begin(), foundWallpapers.end(), back_insert_iterator(deleteFromDb));
    set_difference(foundWallpapers.begin(), foundWallpapers.end(), dbSavedWallpapers.begin(), dbSavedWallpapers.end(), back_insert_iterator(addToDb));

    for (auto& item : deleteFromDb) {
        database.deleteEntry(item);
    }
    for (auto& item : addToDb) {
        Database::Entry entry(item, 0);
        database.addEntry(entry);
    }
}

void update_outputs(Database database) {
    auto nextEntryOptional = database.getNextEntry();

    if (!nextEntryOptional.has_value()) {
        SystemFunctions::messagebox("No wallpapers found!");
        return;
    }

    auto entry = nextEntryOptional.value();

    Database::Entry updatedEntry(entry.path, time_now());

    database.saveEntry(updatedEntry);

    path file(entry.path);

    SystemFunctions::setWallpaper(file);
    SystemFunctions::setLockScreen(file);
}

int WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR    lpCmdLine,
    int       cmdShow)
{
    const int sleep_time = 120;

    FileSource source;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    HANDLE mutex = CreateMutexA(NULL, true, "powers.wallpaperswitcher.singleinstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        SystemFunctions::messagebox("Instance already running!");
        return 1;
    }

    string dbFile = source.getDatabase().string();
    Database database(dbFile);

    while (!is_exited) {
        update_db(database, source);
        update_outputs(database);
        std::this_thread::sleep_for(milliseconds(sleep_time * 1000));
    }

    std::cout << "Goodbye!\n";

    return 0;
}