#pragma once

#include <cstddef>
#include <map>
#include <string>

#include <sqlite3.h>

#include <hanzi-to-pinyin/export/hanzi-to-pinyin_export.h>

#include <hanzi-to-pinyin/config/db_config/db_config.h>

namespace HanZiToPinYin_NS
{
class HANZI_TO_PINYIN_EXPORT PinYinDB
{
public:
    PinYinDB(const DbConfig & db_cfg, size_t max_record_count_per_insert = 10000);

public:
    bool ImportFromHanZiFile(const std::string & hanzi_file_path, const std::string & sqlite_file_path);
    bool ImportFromPhraseFile(const std::string & phrase_file_path, const std::string & sqlite_file_path);

private:
    void ImportFromHanZiString(const std::string & s, sqlite3 * db, const std::string & table_name);
    void ImportFromPhraseString(const std::string & s, sqlite3 * db, const std::string & table_name);
    bool StripToneFromPinyin(const std::string & pinyin_with_tone, std::string & pinyin_without_tone);

private:
    static std::map<wchar_t, char> _map_tone_and_no_tone;
    DbConfig __db_cfg;
    size_t __max_record_count_per_insert;
};
} // namespace HanZiToPinYin_NS
