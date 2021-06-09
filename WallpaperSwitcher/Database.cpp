#include "pch.h"
#include "Database.h"

#include <sstream>
#include <assert.h>
#include "SystemFunctions.h"

using namespace std;

Database::Database(string& dbFile) {
    initConnection(dbFile);
    initTable();
}
Database::~Database() {}

void Database::showSQLErrorAndBail() {
    std::ostringstream msg;
    msg << "sqlite3 error(" << sqlite3_errcode(connection) << "): " << sqlite3_errmsg(connection);
    SystemFunctions::messagebox(msg.str());
    assert(false);
}

void Database::initConnection(string& dbFile) {
    if (sqlite3_open_v2(dbFile.c_str(), &connection, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr)) {
        showSQLErrorAndBail();
    }
}

void Database::initTable() {
    if (sqlite3_exec(connection,
        "CREATE TABLE IF NOT EXISTS wallpapers (FILE TEXT PRIMARY KEY, LASTTS REAL);",
        nullptr,
        nullptr,
        nullptr))
    {
        showSQLErrorAndBail();
    }
}

vector<Database::Entry> Database::getAllEntries() {
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(connection, "SELECT FILE, LASTTS FROM wallpapers;", -1, &stmt, nullptr)) {
        showSQLErrorAndBail();
    }

    vector<Entry> entries;

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        string file(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        double time = sqlite3_column_double(stmt, 1);

        entries.push_back(Entry(move(file), time));
    }
    if (rc != SQLITE_DONE) {
        showSQLErrorAndBail();
    }
    sqlite3_finalize(stmt);

    return entries;
}

optional<Database::Entry> Database::getNextEntry() {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(connection, "SELECT FILE, LASTTS FROM wallpapers ORDER BY LASTTS ASC LIMIT 1;", -1, &stmt, nullptr)) {
        showSQLErrorAndBail();
    }

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        return {};
    }
    else if (rc != SQLITE_ROW) {
        showSQLErrorAndBail();
    }

    string file(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    double time = sqlite3_column_double(stmt, 1);

    sqlite3_finalize(stmt);

    return Entry(move(file), time);
}

void Database::saveEntry(const Database::Entry &entry) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(connection, "UPDATE wallpapers SET LASTTS=? WHERE FILE=?;", -1, &stmt, nullptr)) {
        showSQLErrorAndBail();
    }
    sqlite3_bind_text(stmt, 2, entry.path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 1, entry.time);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        showSQLErrorAndBail();
    }
    sqlite3_finalize(stmt);
}

void Database::addEntry(const Database::Entry &entry) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(connection, "INSERT INTO wallpapers VALUES ( ? , ? );", -1, &stmt, nullptr)) {
        showSQLErrorAndBail();
    }
    sqlite3_bind_text(stmt, 1, entry.path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, entry.time);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        showSQLErrorAndBail();
    }
    sqlite3_finalize(stmt);
}

void Database::deleteEntry(const string& key) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(connection, "DELETE FROM wallpapers WHERE FILE=?;", -1, &stmt, nullptr)) {
        showSQLErrorAndBail();
    }
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        showSQLErrorAndBail();
    }
    sqlite3_finalize(stmt);
}