#include"pch.h"
#include "database.h"
#include <filesystem>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <stdexcept>

// 62进制字符表
const char kShortIdAlphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const int kShortIdBase = 62;

// 将整数转为两位62进制字符串
std::string intToShortId(int n) {
    std::string res(2, '0');
    res[0] = kShortIdAlphabet[(n / kShortIdBase) % kShortIdBase];
    res[1] = kShortIdAlphabet[n % kShortIdBase];
    return res;
}

// 将两位62进制字符串转为整数
int shortIdToInt(const std::string& sid) {
    if (sid.size() != 2) return 0;
    auto pos0 = std::find(std::begin(kShortIdAlphabet), std::end(kShortIdAlphabet), sid[0]);
    auto pos1 = std::find(std::begin(kShortIdAlphabet), std::end(kShortIdAlphabet), sid[1]);
    int idx0 = (pos0 == std::end(kShortIdAlphabet)) ? 0 : (int)(pos0 - kShortIdAlphabet);
    int idx1 = (pos1 == std::end(kShortIdAlphabet)) ? 0 : (int)(pos1 - kShortIdAlphabet);
    return idx0 * kShortIdBase + idx1;
}

// 获取下一个可用短ID，并更新latest
std::string getAndUpdateNextShortId(WordWizServices::Database::DatabaseManager& mainDb)
{
    // 读取latest
    std::string latest = mainDb.executeScalarQuery(
        "SELECT long_id FROM dict_info WHERE short_id = 'latest'"
    );
    int latestInt = 0;
    if (!latest.empty()) {
        latestInt = std::stoi(latest);
    }
    int nextInt = latestInt + 1;
    std::string nextShortId = intToShortId(nextInt);

    // 更新latest
    mainDb.executeQuery(
        "INSERT OR REPLACE INTO dict_info (short_id, long_id) VALUES ('latest', ?)",
        { std::to_string(nextInt) }
    );
    return nextShortId;
}

// 工具函数：合并source_dicts字符串，自动去重
std::string mergeSourceDicts(const std::string& oldDicts, const std::string& newId) {
    std::set<std::string> dictSet;
    if (!oldDicts.empty()) {
        std::stringstream ss(oldDicts);
        std::string id;
        while (std::getline(ss, id, ',')) {
            dictSet.insert(id);
        }
    }
    dictSet.insert(newId);
    std::string result;
    for (const auto& id : dictSet) {
        if (!result.empty()) result += ",";
        result += id;
    }
    return result;
}

// 新增：检测并初始化 main.db
void ensure_main_db_schema(const std::string& mainDbPath) {
    WordWizServices::Database::DatabaseManager mainDb(mainDbPath);

    // 创建 words 表（keyword 唯一，source_dicts 存短ID列表）
    mainDb.executeQuery(
        "CREATE TABLE IF NOT EXISTS words ("
        "keyword TEXT PRIMARY KEY,"
        "source_dicts TEXT NOT NULL)"
    );

    // 创建 dict_info 表（short_id 唯一，long_id 可选）
    mainDb.executeQuery(
        "CREATE TABLE IF NOT EXISTS dict_info ("
        "short_id TEXT PRIMARY KEY,"
        "long_id TEXT)"
    );


    mainDb.executeQuery(
        "INSERT OR IGNORE INTO dict_info (short_id, long_id) VALUES ('latest', '0')"
    );
}



void combine_all_dictionaries_to_main_db(const std::string& mainDbPath, const std::string& dictFolderPath) {
    // 新增：确保 main.db 存在且有表结构
    ensure_main_db_schema(mainDbPath);

    WordWizServices::Database::DatabaseManager mainDb(mainDbPath);

    // 遍历统一目录下所有.db文件
    for (const auto& entry : std::filesystem::directory_iterator(dictFolderPath)) {
        if (entry.path().extension() == ".db" || entry.path().extension() == ".sqlite") {
            std::string dictPath = entry.path().string();
            WordWizServices::Database::DatabaseManager dictDb(dictPath);

            // 获取词典ID
            std::string dictId = dictDb.executeScalarQuery("SELECT AttributeValue FROM info WHERE AttributeName = 'ID'");
            if (dictId.empty()) continue;

            // 在插入前先查找是否已存在
            std::string existShortId = mainDb.executeScalarQuery(
                "SELECT short_id FROM dict_info WHERE long_id = ?",
                { dictId }
            );

            std::string newShortId;
            if (!existShortId.empty()) {
                newShortId = existShortId;
            }
            else {
                newShortId = getAndUpdateNextShortId(mainDb);
                mainDb.executeQuery(
                    "INSERT INTO dict_info (short_id, long_id) VALUES (?, ?)",
                    { newShortId, dictId }
                );
            }

            // 使用ATTACH将子库附加到主库
            std::string attachSql = "ATTACH DATABASE ? AS subdict";
            mainDb.executeQuery(attachSql, { dictPath });

            // 用单条SQL完成合并（自动去重，追加ID）
            // 1. 插入新词
            mainDb.executeQuery(
                "INSERT INTO words (keyword, source_dicts) "
                "SELECT keyword, ? FROM subdict.word "
                "WHERE keyword NOT IN (SELECT keyword FROM words)",
                { newShortId }
            );

            // 2. 已有词，追加ID（如果没有则直接赋值，有则追加，去重可用自定义函数或简单拼接）
            // 这里简单拼接，后续可用正则或自定义函数去重
            mainDb.executeQuery(
                "UPDATE words SET source_dicts = "
                "CASE "
                "  WHEN instr(',' || source_dicts || ',', ',' || ? || ',') = 0 "
                "  THEN source_dicts || ',' || ? "
                "  ELSE source_dicts "
                "END "
                "WHERE keyword IN (SELECT keyword FROM subdict.word)",
                { newShortId, newShortId }
            );

            // 分离子库
            mainDb.executeQuery("DETACH DATABASE subdict");
        }
    }
}

// 只合并新导入的词典到 main.db
void combine_new_dictionaries_to_main_db(const std::string& mainDbPath, const std::vector<std::string>& newDictPaths) {
    // 确保 main.db 存在且有表结构
    ensure_main_db_schema(mainDbPath);

    WordWizServices::Database::DatabaseManager mainDb(mainDbPath);

    for (const auto& dictPath : newDictPaths) {
        if (!(dictPath.size() > 3 && 
            (dictPath.substr(dictPath.size() - 3) == ".db" || dictPath.substr(dictPath.size() - 7) == ".sqlite"))) {
            continue; // 只处理 .db 或 .sqlite 文件
        }

        WordWizServices::Database::DatabaseManager dictDb(dictPath);

        // 获取词典ID
        std::string dictId = dictDb.executeScalarQuery("SELECT AttributeValue FROM info WHERE AttributeName = 'ID'");
        if (dictId.empty()) continue;

        // 在插入前先查找是否已存在
        std::string existShortId = mainDb.executeScalarQuery(
            "SELECT short_id FROM dict_info WHERE long_id = ?",
            { dictId }
        );

        std::string newShortId;
        if (!existShortId.empty()) {
            newShortId = existShortId;
        }
        else {
            newShortId = getAndUpdateNextShortId(mainDb);
            mainDb.executeQuery(
                "INSERT INTO dict_info (short_id, long_id) VALUES (?, ?)",
                { newShortId, dictId }
            );
        }

        // 使用ATTACH将子库附加到主库
        std::string attachSql = "ATTACH DATABASE ? AS subdict";
        mainDb.executeQuery(attachSql, { dictPath });

        // 1. 插入新词
        mainDb.executeQuery(
            "INSERT INTO words (keyword, source_dicts) "
            "SELECT keyword, ? FROM subdict.word "
            "WHERE keyword NOT IN (SELECT keyword FROM words)",
            { newShortId }
        );

        // 2. 已有词，追加ID
        mainDb.executeQuery(
            "UPDATE words SET source_dicts = "
            "CASE "
            "  WHEN instr(',' || source_dicts || ',', ',' || ? || ',') = 0 "
            "  THEN source_dicts || ',' || ? "
            "  ELSE source_dicts "
            "END "
            "WHERE keyword IN (SELECT keyword FROM subdict.word)",
            { newShortId, newShortId }
        );

        // 分离子库
        mainDb.executeQuery("DETACH DATABASE subdict");
    }
}