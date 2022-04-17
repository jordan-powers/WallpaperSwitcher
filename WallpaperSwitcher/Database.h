#pragma once
#include <string>
#include <vector>
#include <optional>

#include <sqlite3.h>

class Database {
public:
    Database(const std::string& dbFile);
    virtual ~Database();

    struct Entry {
        const std::string path;
        const double time;

        Entry(std::string path, double time)
            : path{move(path)}
            , time{time}
        {}
    };

    std::vector<Entry> getAllEntries();
    std::optional<Entry> getNextEntry();
    
    void saveEntry(const Entry&);
    void deleteEntry(const std::string&);
    void addEntry(const Entry&);

private:
    void initConnection(const std::string& dbFile);
    void initTable();

    void showSQLErrorAndBail();

    sqlite3* connection{ nullptr };
};

