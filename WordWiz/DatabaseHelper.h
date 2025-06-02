#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>

class DatabaseHelper
{
public:
    bool OpenDatabase(const wchar_t* dbPath);
    std::vector<std::wstring> QueryWords(const wchar_t* query);
    ~DatabaseHelper();

private:
    sqlite3* db = nullptr;
};