#include "DatabaseHelper.h"

bool DatabaseHelper::OpenDatabase(const wchar_t* dbPath)
{
    int result = sqlite3_open16(dbPath, &db);
    return result == SQLITE_OK;
}

std::vector<std::wstring> DatabaseHelper::QueryWords(const wchar_t* query)
{
    // 复制之前提供的QueryWords方法实现...
}

DatabaseHelper::~DatabaseHelper()
{
    if (db) sqlite3_close(db);
}