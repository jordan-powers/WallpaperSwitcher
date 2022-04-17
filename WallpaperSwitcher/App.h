#pragma once

#include "FileSource.h"
#include "Database.h"

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

    App();

    FileSource m_source;
    Database m_database;

    void update_db();
    void update_outputs();
};