#include <hanzi-to-pinyin/pch/pch.h>

#include <hanzi-to-pinyin/config/db_config/db_config.h>

using namespace HanZiToPinYin_NS;

DbConfig::DbConfig(const DbConfig & cfg)
{
    __sqlite_file_path = cfg.__sqlite_file_path;
    __table_name_for_char = cfg.__table_name_for_char;
    __table_name_for_phrase = cfg.__table_name_for_phrase;
}
