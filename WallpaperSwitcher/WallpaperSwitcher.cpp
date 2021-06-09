#include "pch.h"
#include <winrt/base.h>
#include <iostream>
#include <sqlite3.h>
#include <stdlib.h>
#include <filesystem>
#include <chrono>
#include <thread>
#include <Windows.h>
#include <csignal>
#include <sstream>
#include <winrt/Windows.System.UserProfile.h>
#include <winrt/Windows.Storage.h>

#include "SystemFunctions.h"
#include "FileSource.h"

using namespace std::filesystem;
using namespace std::chrono;

using namespace winrt;
using namespace Windows::Storage;
using namespace Windows::System::UserProfile;

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

void error_out(std::string msg) {
    wchar_t* wmsg = new wchar_t[msg.length() + 1];
    mbstowcs_s(NULL, wmsg, msg.length() + 1, msg.c_str(), _TRUNCATE);
    MessageBox(
        NULL,
        wmsg,
        L"WallpaperSwitcher Error",
        MB_OK
    );
    delete[] wmsg;
    exit(EXIT_FAILURE);
}

void update_db(sqlite3* db, const FileSource source) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, "SELECT FILE FROM wallpapers;", -1, &stmt, nullptr)) {
        std::ostringstream msg;
        msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
        error_out(msg.str());
    }
    std::list<std::string> tracked_files;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::string row((char*)sqlite3_column_text(stmt, 0));
        tracked_files.push_back(row);
    }
    if (rc != SQLITE_DONE) {
        std::ostringstream msg;
        msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
        error_out(msg.str());
    }
    sqlite3_finalize(stmt);

    auto wallpapers = source.getWallpapers();

    for (auto& item : wallpapers) {
        if (std::find(tracked_files.begin(), tracked_files.end(), item.string()) != tracked_files.end()) {
            tracked_files.remove(item.string());
        }
        else {
            std::cout << "Adding " << item << "\n";
            if (sqlite3_prepare_v2(db, "INSERT INTO wallpapers VALUES ( ? , ? );", -1, &stmt, nullptr)) {
                std::ostringstream msg;
                msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
                error_out(msg.str());
            }
            sqlite3_bind_text(stmt, 1, item.string().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt, 2, 0);
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
                std::ostringstream msg;
                msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
                error_out(msg.str());
            }
            sqlite3_finalize(stmt);
        }
    }

    for (auto& p : tracked_files) {
        std::cout << "Removing " << p << "\n";
        if (sqlite3_prepare_v2(db, "DELETE FROM wallpapers WHERE FILE=?;", -1, &stmt, nullptr)) {
            std::ostringstream msg;
            msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
            error_out(msg.str());
        }
        sqlite3_bind_text(stmt, 1, p.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
            std::ostringstream msg;
            msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
            error_out(msg.str());
        }
        sqlite3_finalize(stmt);
    }
}

void set_wallpaper(path wallpaper) {
    SystemParametersInfoA(20, 0, (PVOID)wallpaper.string().c_str(), 0);
}

void set_lockscreen(path lockscreen) {
    std::string lstring = lockscreen.string();
    wchar_t* wlstring = new wchar_t[lstring.length() + 1];
    mbstowcs_s(NULL, wlstring, lstring.length() + 1, lstring.c_str(), _TRUNCATE);

    StorageFile file = StorageFile::GetFileFromPathAsync(wlstring).get();
    LockScreen::SetImageFileAsync(file).get();

    delete[] wlstring;
}

void update_outputs(sqlite3 *db) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, "SELECT FILE FROM wallpapers ORDER BY LASTTS ASC LIMIT 1;", -1, &stmt, nullptr)) {
        std::ostringstream msg;
        msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
        error_out(msg.str());
    }
    int rc;
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        if (rc == SQLITE_DONE) {
            error_out("No wallpapers found!\n");
        }
        else {
            std::ostringstream msg;
            msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
            error_out(msg.str());
        }
        exit(EXIT_FAILURE);
    }
    std::string filestr((char*)sqlite3_column_text(stmt, 0));
    sqlite3_finalize(stmt);

    if (sqlite3_prepare_v2(db, "UPDATE wallpapers SET LASTTS=? WHERE FILE=?;", -1, &stmt, nullptr)) {
        std::ostringstream msg;
        msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
        error_out(msg.str());
    }
    sqlite3_bind_text(stmt, 2, filestr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 1, time_now());
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        std::ostringstream msg;
        msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
        error_out(msg.str());
    }
    sqlite3_finalize(stmt);

    path file(filestr);

    set_wallpaper(file);
    set_lockscreen(file);

}

int WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR    lpCmdLine,
    int       cmdShow)
{
    const int sleep_time = 120;

    FileSource source;

    sqlite3 *db;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    HANDLE mutex = CreateMutexA(NULL, true, "powers.wallpaperswitcher.singleinstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        error_out("Instance already running!");
    }

    int rc;

    rc = sqlite3_open_v2(source.getDatabase().string().c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc) {
        std::ostringstream msg;
        msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
        error_out(msg.str());
    }

    rc = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS wallpapers (FILE TEXT PRIMARY KEY, LASTTS REAL);",
        nullptr,
        nullptr,
        nullptr);
    if (rc) {
        std::ostringstream msg;
        msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
        error_out(msg.str());
    }

    while (!is_exited) {
        update_db(db, source);
        update_outputs(db);
        std::this_thread::sleep_for(milliseconds(sleep_time * 1000));
    }

    std::cout << "Goodbye!\n";

    return 0;
}