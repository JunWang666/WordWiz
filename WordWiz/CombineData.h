#pragma once

#include <string>

// ÉùÃ÷ÈÚºÏº¯Êı
void combine_all_dictionaries_to_main_db(const std::string& mainDbPath, const std::string& dictFolderPath);
void combine_new_dictionaries_to_main_db(const std::string& mainDbPath, const std::vector<std::string>& newDictPaths);