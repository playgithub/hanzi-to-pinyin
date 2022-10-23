#include <stdexcept>

#include <hanzi-to-pinyin/pch/pch.h>
#include <hanzi-to-pinyin/config/config.h>

#include "pinyin_db.h"

using namespace HanZiToPinYin_NS;

std::map<wchar_t, char> PinYinDB::_map_tone_and_no_tone = {
    {L'ā', 'a'}, {L'á', 'a'}, {L'ǎ', 'a'}, {L'à', 'a'}, {L'ē', 'e'}, {L'é', 'e'}, {L'ě', 'e'}, {L'è', 'e'}, {L'ī', 'i'},
    {L'í', 'i'}, {L'ǐ', 'i'}, {L'ì', 'i'}, {L'ō', 'o'}, {L'ó', 'o'}, {L'ǒ', 'o'}, {L'ò', 'o'}, {L'ū', 'u'}, {L'ú', 'u'},
    {L'ǔ', 'u'}, {L'ù', 'u'}, {L'ǜ', 'v'}, {L'ǘ', 'v'}, {L'ǚ', 'v'}, {L'ǜ', 'v'}, {L'ü', 'v'}
    //{L'm̄','m'},
    //{L'ḿ','m'},
    //{L'm̀','m'},
    //{L'ń','n'},
    //{L'ň','n'},
    //{L'ǹ','n'},
    //{L'ế','e'},
    //{L'ê','e'},
    //{L'ề','e'},
    //{L'ê̄','e'},
    //{L'ê̌','e'}
};

PinYinDB::PinYinDB(const DbConfig & db_cfg, size_t max_record_count_per_insert)
    : __db_cfg(db_cfg), __max_record_count_per_insert(max_record_count_per_insert)
{
}

bool PinYinDB::ImportFromHanZiFile(const std::string & hanzi_file_path, const std::string & sqlite_file_path)
{
    bool ok = false;

    sqlite3 * db = nullptr;
    if (sqlite3_open_v2(sqlite_file_path.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) ==
            SQLITE_OK)
    {
        std::ifstream ifs;
        ifs.open(hanzi_file_path);
        if (!ifs.fail())
        {
            std::string s((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

            if (ImportFromHanZiString(s, db, __db_cfg.GetTableNameForChar()))
                ok = true;
            else
                
                std::cerr << "FromStringForHanZi failed\n";

            ifs.close();
        }
        else
            std::cerr << "failed to open plain file for pinyin\n";

        if (sqlite3_close_v2(db) != SQLITE_OK)
        {
            std::cerr << sqlite3_errmsg(db) << '\n';
            ok = false;
        }
    }
    else
    {
        std::cerr << sqlite3_errmsg(db) << '\n';
        if (db != nullptr && sqlite3_close_v2(db) != SQLITE_OK)
            std::cerr << sqlite3_errmsg(db) << '\n';
    }

    return ok;
}

bool PinYinDB::ImportFromPhraseFile(const std::string & phrase_file_path, const std::string & sqlite_file_path)
{
    bool ok = false;

    sqlite3 * db = nullptr;
    if (sqlite3_open_v2(sqlite_file_path.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) ==
            SQLITE_OK)
    {
        std::ifstream ifs;
        ifs.open(phrase_file_path);
        if (!ifs.fail())
        {
            std::string s((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

            if (ImportFromPhraseString(s, db, __db_cfg.GetTableNameForPhrase()))
                ok = true;
            else
                std::cerr << "FromStringForPhrase failed\n";

            ifs.close();
        }
        else
            std::cerr << "failed to open plain file for pinyin\n";

        if (sqlite3_close_v2(db) != SQLITE_OK)
        {
            std::cerr << sqlite3_errmsg(db) << '\n';
            ok = false;
        }
    }
    else
    {
        std::cerr << sqlite3_errmsg(db) << '\n';
        if (db != nullptr && sqlite3_close_v2(db) != SQLITE_OK)
            std::cerr << sqlite3_errmsg(db) << '\n';
    }

    return ok;
}

bool PinYinDB::ImportFromHanZiString(const std::string & s, sqlite3 * db, const std::string & table_name)
{
    std::string::const_iterator it_begin = s.cbegin();
    std::string::const_iterator it_end = s.cend();

    std::ostringstream sql;

    char * error_msg = nullptr;

    sql << "CREATE TABLE IF NOT EXISTS " << table_name << "(hanzi_utf16 INTEGER PRIMARY KEY NOT NULL,"
        << "py_no_tone NVARCHAR(6) NOT NULL,"
        << "py_tone NVARCHAR(6) NOT NULL)";

    if (sqlite3_exec(db, sql.str().c_str(), nullptr, 0, &error_msg) != SQLITE_OK)
    {
        std::cerr << error_msg << '\n';
        sqlite3_free(error_msg);
        error_msg = nullptr;
        return false;
    }

    sql.str("");
    sql.clear();

    sql << "INSERT INTO " << table_name << " VALUES";

    bool ok = true;
    bool has_item = false;

    std::string s_pinyins_tone;
    std::vector<std::string> v_pinyins_tone;
    std::vector<std::string> v_pinyins_no_tone;

    std::string s_pinyin_no_tone;

    std::ostringstream oss;
    std::regex regex("U\\+(.+?):\\s*?(\\S+?)[\\s#]");
    std::smatch match;

    while (std::regex_search(it_begin, it_end, match, regex))
    {
        char * endptr = nullptr;
        std::uint32_t utf8_code = std::strtoul(match[1].str().c_str(), &endptr, 16);
        if (*endptr != 0)
        {
            std::cerr << "strtoul not perfect\n";
            ok = false;
            break;
        }

        s_pinyins_tone = match[2].str();

        boost::split(v_pinyins_tone, s_pinyins_tone, boost::is_any_of(","));
        v_pinyins_no_tone.clear();

        for (const std::string & s_pinyin_tone : v_pinyins_tone)
        {
            if (StripToneFromPinyin(s_pinyin_tone, s_pinyin_no_tone))
                v_pinyins_no_tone.push_back(s_pinyin_no_tone);
        }

        if (!v_pinyins_no_tone.empty())
        {
            if (has_item)
                sql << ',';
            else
                has_item = true;

            sql << '(' << utf8_code << ",'" << boost::join(v_pinyins_no_tone, ",") << "','" << s_pinyins_tone << "')";
        }

        it_begin = match.suffix().first;
    }

    sql << " ON CONFLICT(hanzi_utf16) DO UPDATE SET py_no_tone=excluded.py_no_tone, py_tone=excluded.py_tone";

    if (ok)
    {
        if (sqlite3_exec(db, sql.str().c_str(), nullptr, 0, &error_msg) != SQLITE_OK)
        {
            std::cerr << error_msg << '\n';
            sqlite3_free(error_msg);
            error_msg = nullptr;
            ok = false;
        }
    }

    return ok;
}

bool PinYinDB::ImportFromPhraseString(const std::string & s, sqlite3 * db, const std::string & table_name)
{
    std::ostringstream oss_sql;

    char * error_msg = nullptr;

    oss_sql << "CREATE TABLE IF NOT EXISTS " << table_name << "(phrase_utf8 TEXT PRIMARY KEY NOT NULL,"
            << "py_no_tone TEXT NOT NULL,"
            << "py_tone TEXT NOT NULL)";

    if (sqlite3_exec(db, oss_sql.str().c_str(), nullptr, 0, &error_msg) != SQLITE_OK)
    {
        std::cerr << error_msg << '\n';
        sqlite3_free(error_msg);
        error_msg = nullptr;
        return false;
    }

    std::ostringstream oss_sql_head;
    oss_sql_head << "INSERT INTO " << table_name << " VALUES";
    std::string sql_head(oss_sql_head.str());

    std::ostringstream oss_sql_values;

    bool ok = true;
    std::string s_pinyins_tone;
    std::string s_pinyins_no_tone;

    std::regex regex("(?!#)(^.+?):\\s*?(\\S[\\S ]+?)[\r\n]+");
    std::smatch match;

    std::string::const_iterator it_begin = s.cbegin();
    std::string::const_iterator it_end = s.cend();

    size_t record_count_to_insert = 0;

    std::set<std::string> set;

    while (std::regex_search(it_begin, it_end, match, regex))
    {
        s_pinyins_tone = match[2].str();

        if (StripToneFromPinyin(s_pinyins_tone, s_pinyins_no_tone))
        {
            if (record_count_to_insert > 0)
                oss_sql_values << ",";

            set.insert(match[1].str());

            oss_sql_values << "('" << match[1].str() << "','" << s_pinyins_no_tone << "','" << s_pinyins_tone << "')";

            ++record_count_to_insert;

            if (record_count_to_insert >= __max_record_count_per_insert)
            {
                oss_sql.str("");
                oss_sql.clear();

                oss_sql << sql_head << oss_sql_values.str()
                        << " ON CONFLICT(phrase_utf8) DO UPDATE SET py_no_tone=excluded.py_no_tone, "
                           "py_tone=excluded.py_tone";

                if (sqlite3_exec(db, oss_sql.str().c_str(), nullptr, 0, &error_msg) != SQLITE_OK)
                {
                    std::cerr << error_msg << '\n';
                    sqlite3_free(error_msg);
                    error_msg = nullptr;
                    ok = false;
                    break;
                }

                record_count_to_insert = 0;

                oss_sql_values.str("");
                oss_sql_values.clear();
            }
        }

        it_begin = match.suffix().first;
    }

    if (ok && record_count_to_insert > 0)
    {
        oss_sql.str("");
        oss_sql.clear();

        oss_sql << sql_head << oss_sql_values.str()
                << " ON CONFLICT(phrase_utf8) DO UPDATE SET py_no_tone=excluded.py_no_tone, py_tone=excluded.py_tone";

        if (sqlite3_exec(db, oss_sql.str().c_str(), nullptr, 0, &error_msg) != SQLITE_OK)
        {
            std::cerr << error_msg << '\n';
            sqlite3_free(error_msg);
            error_msg = nullptr;
            ok = false;
        }
    }

    return ok;
}

bool PinYinDB::StripToneFromPinyin(const std::string & pinyin_with_tone, std::string & pinyin_without_tone)
{
    bool ok = true;

    std::wstring pinyin_tone_unicode = boost::locale::conv::utf_to_utf<wchar_t>(pinyin_with_tone);
    std::map<wchar_t, char>::const_iterator it;
    std::ostringstream oss;
    for (wchar_t wc : pinyin_tone_unicode)
    {
        it = _map_tone_and_no_tone.find(wc);
        if (it == _map_tone_and_no_tone.cend())
        {
            if ((wc >= 'a' && wc <= 'z') || wc == L',' || wc == L' ' || (wc >= 'A' && wc <= 'Z'))
                oss << static_cast<char>(wc);
            else
            {
                ok = false;
                break;
            }
        }
        else
            oss << it->second;
    }

    if (ok)
        pinyin_without_tone = oss.str();

    return ok;
}
