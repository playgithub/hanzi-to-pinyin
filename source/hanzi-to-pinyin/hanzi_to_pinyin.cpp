#include <hanzi-to-pinyin/pch/pch.h>

#include "hanzi_to_pinyin.h"

using namespace HanZiToPinYin_NS;

HanZiToPinYin::HanZiToPinYin() :
    __db(nullptr)
{
}

HanZiToPinYin::~HanZiToPinYin()
{
    UnInit();
}

void HanZiToPinYin::Init(const Config & config, const cppjieba::config & cppjieba_config)
{
    UnInit();

    const DbConfig & db_config = config.GetDbConfig();

    if (sqlite3_open_v2(db_config.GetSqliteFilePath().c_str(), &__db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK)
        throw std::runtime_error("sqlite3_open_v2 failed");

    __jieba.reset(new cppjieba::Jieba(cppjieba_config.get_dict_path_jieba_dict(),
                                      cppjieba_config.get_dict_path_hmm_model(),
                                      cppjieba_config.get_dict_path_user_dict(),
                                      cppjieba_config.get_dict_path_idf(),
                                      cppjieba_config.get_dict_path_stop_words()));

    __config = config;
}

void HanZiToPinYin::UnInit()
{
    if (__db != nullptr)
    {
        if (sqlite3_close_v2(__db) == SQLITE_OK)
            __db = nullptr;
        else
            throw std::runtime_error("sqlite3_close_v2 failed");
    }

    __jieba.reset();
}

void HanZiToPinYin::GetPinYin(const std::string & input_in_utf8,
                              std::string * pinyin_with_tone,
                              std::string * pinyin_without_tone,
                              std::string * pinyin_first_letters,
                              const char seperator)
{
    if (pinyin_with_tone != nullptr)
    {
        GetPinYin(input_in_utf8, *pinyin_with_tone, true, seperator);
    }

    if (pinyin_without_tone != nullptr || pinyin_first_letters != nullptr)
    {
        std::string pinyin_without_tone_tmp;
        GetPinYin(input_in_utf8, pinyin_without_tone_tmp, false, seperator);

        if (pinyin_first_letters != nullptr)
        {
            *pinyin_first_letters = GetFirstLettersOfPinYin(pinyin_without_tone_tmp, seperator);
        }

        if (pinyin_without_tone != nullptr)
        {
            *pinyin_without_tone = std::move(pinyin_without_tone_tmp);
        }
    }
}

void HanZiToPinYin::GetPinYin(const std::string & input_in_utf8,
                              std::string & output_in_utf8,
                              bool with_tone,
                              const char seperator)
{
    std::vector<std::string> tokens;
    std::string result;

    std::ostringstream oss_output;

    bool ok = false;
    { // try sophisticated cut
        __jieba->Cut(input_in_utf8, tokens, true);
        bool has_item = false;
        std::string pinyin;
        ok = true;
        for (const std::string & token : tokens)
        {
            if (GetPhrasePinYin(token, pinyin, with_tone, seperator))
            {
                if (has_item)
                    oss_output << seperator;
                else
                    has_item = true;
                oss_output << pinyin;
            }
            else
            {
                ok = false;
                break;
            }
        }
    }

    if (ok)
    {
        output_in_utf8 = std::move(oss_output.str());
        return;
    }

    oss_output.clear();
    oss_output.str("");

    { // try hmm cut
        __jieba->CutHMM(input_in_utf8, tokens);
        bool has_item = false;
        std::string pinyin;
        std::wstring wtoken;
        std::vector<std::string> han_zi_pinyins;

        for (const std::string & token : tokens)
        {
            wtoken = boost::locale::conv::utf_to_utf<wchar_t>(token);
            if (wtoken.size() > 1)
            {
                if (GetPhrasePinYin(token, pinyin, with_tone, seperator))
                {
                    if (has_item)
                        oss_output << seperator;
                    else
                        has_item = true;
                    oss_output << pinyin;
                }
                else
                {
                    for (const wchar_t & wc : wtoken)
                    {
                        if (GetCharPinYin(wc, han_zi_pinyins, with_tone, seperator))
                        {
                            if (has_item)
                                oss_output << seperator;
                            else
                                has_item = true;
                            oss_output << han_zi_pinyins[0];
                        }
                        else
                        {
                            if (has_item)
                                oss_output << seperator;
                            else
                                has_item = true;
                            oss_output << boost::locale::conv::utf_to_utf<char>(std::wstring(1, wc));
                        }
                    }
                }
            }
            else
            {
                assert(wtoken.size() == 1);
                if (GetCharPinYin(wtoken.front(), han_zi_pinyins, with_tone, seperator))
                {
                    if (has_item)
                        oss_output << seperator;
                    else
                        has_item = true;
                    oss_output << han_zi_pinyins[0];
                }
                else
                {
                    if (has_item)
                        oss_output << seperator;
                    else
                        has_item = true;
                    oss_output << boost::locale::conv::utf_to_utf<char>(std::wstring(1, wtoken.front()));
                }
            }
        }
    }

    output_in_utf8 = std::move(oss_output.str());
}

void HanZiToPinYin::GetPinYinFirstLetters(const std::string & input_in_utf8,
                                          std::string & output_in_utf8,
                                          const char seperator)
{
    std::string pinyin;

    GetPinYin(input_in_utf8, pinyin);

    output_in_utf8 = GetFirstLettersOfPinYin(pinyin, seperator);
}

bool HanZiToPinYin::GetPhrasePinYin(const std::string & phrase,
                                    std::string & phrase_pinyin,
                                    bool with_tone,
                                    const char seperator)
{
    std::ostringstream oss_sql;
    if (with_tone)
        oss_sql << "SELECT py_tone FROM ";
    else
        oss_sql << "SELECT py_no_tone FROM ";
    oss_sql << __config.GetDbConfig().GetTableNameForPhrase() << " WHERE phrase_utf8='" << phrase << "' LIMIT 1";

    bool ok = false;

    char * error_msg = nullptr;
    phrase_pinyin.clear();
    if (sqlite3_exec(__db, oss_sql.str().c_str(), &HanZiToPinYin::Callback_Phrase, &phrase_pinyin, &error_msg) == SQLITE_OK)
    {
        if (!phrase_pinyin.empty())
            ok = true;
    }
    else
    {
        std::ostringstream oss;
        oss << u8"sqlite3_exec failed (" << error_msg << u8')';
        
        sqlite3_free(error_msg);
        
        throw std::runtime_error(oss.str());
    }

    return ok;
}

bool HanZiToPinYin::GetCharPinYin(const wchar_t wc,
                                  std::vector<std::string> & pinyins,
                                  bool with_tone,
                                  const char seperator)
{
    pinyins.clear();

    if (wc >= 0x20 && wc <= 0x7e)
    {
        char c = static_cast<char>(wc);
        pinyins.push_back({c});
        return true;
    }
    else if (wc >= 0xff21 && wc <= 0xff3a) // ��-��
    {
        //char c = char(wc - 0xff21 + 'A');
        char c = char(wc - 0xff21 + 'a');
        pinyins.push_back({c});
        return true;
    }
    else if (wc >= 0xff41 && wc <= 0xff5a) // ��-��
    {
        char c = char(wc - 0xff41 + 'a');
        pinyins.push_back({c});
        return true;
    }

    std::ostringstream oss_sql;
    if (with_tone)
        oss_sql << "SELECT py_tone FROM ";
    else
        oss_sql << "SELECT py_no_tone FROM ";
    oss_sql << __config.GetDbConfig().GetTableNameForChar() << " WHERE hanzi_utf16=" << std::uint16_t(wc) << " LIMIT 1";

    bool ok = false;

    char * error_msg = nullptr;
    pinyins.clear();
    if (sqlite3_exec(__db, oss_sql.str().c_str(), &HanZiToPinYin::Callback_HanZi, &pinyins, &error_msg) == SQLITE_OK)
    {
        ok = !pinyins.empty();
    }
    else
    {
        std::ostringstream oss;
        oss << u8"sqlite3_exec failed (" << error_msg << u8')';

        sqlite3_free(error_msg);

        throw std::runtime_error(oss.str());
    }

    return ok;
}

std::string HanZiToPinYin::GetFirstLettersOfPinYin(const std::string & pinyin, const char seperator) const
{
    std::string first_letters;

    std::string seperators{seperator};

    std::vector<std::string> v;
    boost::split(v, pinyin, boost::is_any_of(seperators), boost::token_compress_on);
    first_letters.reserve(v.size());
    for (const std::string & s : v)
    {
        if (!s.empty())
        {
            char c = s.front();
            if (c >= 'a' && c <= 'z')
                first_letters.push_back(c);
            else if (c >= 'A' && c <= 'Z')
                first_letters.push_back(c - 'A' + 'a');
        }
    }

    return first_letters;
}

int HanZiToPinYin::Callback_Phrase(void * data, int nCols, char ** ppColValue, char ** ppColName)
{
    std::string * pinyin_without_tone = reinterpret_cast<std::string *>(data);
    if (ppColValue[0] != nullptr)
    {
        *pinyin_without_tone = ppColValue[0];
    }
    else
        pinyin_without_tone->clear();
    return SQLITE_OK;
}

int HanZiToPinYin::Callback_HanZi(void * data, int nCols, char ** ppColValue, char ** ppColName)
{
    std::vector<std::string> * pinyins = reinterpret_cast<std::vector<std::string> *>(data);

    if (ppColValue[0] != nullptr)
        boost::split(*pinyins, ppColValue[0], boost::is_any_of(", "), boost::token_compress_on);
    else
        pinyins->clear();

    return SQLITE_OK;
}
