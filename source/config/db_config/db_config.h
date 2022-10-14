#pragma once

#include <string>

namespace HanZiToPinYin_NS
{
class DbConfig
{
public:
    DbConfig() {}
    DbConfig(const DbConfig & cfg);

public:
    inline void SetSqliteFilePath(const std::string & path) { __sqlite_file_path = path; }
    inline void SetSqliteFileath(std::string && path) { __sqlite_file_path = std::move(path); }

    inline void SetTableNameForChar(const std::string & name) { __table_name_for_char = name; }
    inline void SetTableNameForChar(std::string && name) { __table_name_for_char = std::move(name); }

    inline void SetTableNameForPhrase(const std::string & name) { __table_name_for_phrase = name; }
    inline void SetTableNameForPhrase(std::string && name) { __table_name_for_phrase = std::move(name); }

    inline const std::string & GetSqliteFilePath() const { return __sqlite_file_path; }

    inline const std::string & GetTableNameForChar() const { return __table_name_for_char; }

    inline const std::string & GetTableNameForPhrase() const { return __table_name_for_phrase; }

private:
    std::string __sqlite_file_path;
    std::string __table_name_for_char;
    std::string __table_name_for_phrase;
};
} // namespace HanZiToPinYin_NS
