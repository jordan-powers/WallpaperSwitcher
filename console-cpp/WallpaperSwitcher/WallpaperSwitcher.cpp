// WallpaperSwitcher.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <sqlite3.h>
#include <stdlib.h>
#include <filesystem>
#include <chrono>
#include <thread>
#include <Windows.h>
#include <csignal>
#include <sstream>

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

void error_out(std::string msg) {
    wchar_t* wmsg = new wchar_t[msg.length() + 1];
    mbstowcs_s(NULL, wmsg, msg.length() + 1, msg.c_str(), _TRUNCATE);
    MessageBox(
        NULL,
        wmsg,
        L"WallpaperManager Error",
        MB_OK
    );
    delete[] wmsg;
    exit(EXIT_FAILURE);
}

void update_db(sqlite3* db, const path find_path, const std::vector<std::string> ext) {
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

    for (auto& p : recursive_directory_iterator(find_path)) {
        auto item_path = p.path();
        if (item_path.has_extension()) {
            if (std::find(ext.begin(), ext.end(), item_path.extension().string()) != ext.end()) {
                if (std::find(tracked_files.begin(), tracked_files.end(), item_path.string()) != tracked_files.end()) {
                    tracked_files.remove(item_path.string());
                }
                else {
                    std::cout << "Adding " << item_path << "\n";
                    if (sqlite3_prepare_v2(db, "INSERT INTO wallpapers VALUES ( ? , ? );", -1, &stmt, nullptr)) {
                        std::ostringstream msg;
                        msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
                        error_out(msg.str());
                    }
                    sqlite3_bind_text(stmt, 1, item_path.string().c_str(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_double(stmt, 2, time_now());
                    rc = sqlite3_step(stmt);
                    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
                        std::ostringstream msg;
                        msg << "sqlite3 error(" << sqlite3_errcode(db) << "): " << sqlite3_errmsg(db);
                        error_out(msg.str());
                    }
                    sqlite3_finalize(stmt);
                }
            }
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

void set_wallpaper(sqlite3 *db) {
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

    std::cout << "Setting wallpaper to " << file.filename() << "\n";

    SystemParametersInfoA(20, 0, (PVOID) file.string().c_str(), 0);
}

int WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR    lpCmdLine,
    int       cmdShow)
{
    char* buff;
    size_t numelem;
    const char* _exts[] = { ".jpg", ".png" };
    const int sleep_time = 120;
    std::vector<std::string> exts(_exts, _exts + (sizeof(_exts) / sizeof(char*)));

    _dupenv_s(&buff, &numelem, "USERPROFILE");
    path wallpaper_folder(buff);
    free(buff);
    
    wallpaper_folder /= "Documents\\wallpapers";

    path database_path = wallpaper_folder / "wallpapers.db";
    sqlite3 *db;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    HANDLE mutex = CreateMutexA(NULL, true, "powers.wallpapermanager.singleinstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        error_out("Instance already running!");
    }

    int rc;

    rc = sqlite3_open_v2(database_path.string().c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
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
        update_db(db, wallpaper_folder, exts);
        set_wallpaper(db);
        std::this_thread::sleep_for(milliseconds(sleep_time * 1000));
    }

    std::cout << "Goodbye!\n";

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
