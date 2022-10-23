#pragma once

#include <memory>
#include <vector>
#include <sqlite3.h>
#include <cppjieba/config.hpp>
#include <cppjieba/Jieba.hpp>
#include <hanzi-to-pinyin/export/hanzi-to-pinyin_export.h>
#include <hanzi-to-pinyin/config/config.h>

namespace HanZiToPinYin_NS
{
class HANZI_TO_PINYIN_EXPORT HanZiToPinYin
{
public:
    HanZiToPinYin();
    ~HanZiToPinYin();

public:
    void InitDefault()
    {
        Config config;
        config.LoadDefault();

        cppjieba::config cppjieba_config;
        cppjieba_config.load_default();

        Init(config, cppjieba_config);
    }
    void Init(const Config & config, const cppjieba::config & cppjieba_config);
    void UnInit();

public:
    void GetPinYin(const std::string & input_in_utf8,
                   std::string * pinyin_with_tone,
                   std::string * pinyin_without_tone,
                   std::string * pinyin_first_letters,
                   const char seperator = ' ');

private:
    void GetPinYin(const std::string & input_in_utf8,
                   std::string & output_in_utf8,
                   bool with_tone = false,
                   const char seperator = ' ');

    void GetPinYinFirstLetters(const std::string & input_in_utf8,
                               std::string & output_in_utf8,
                               const char seperator = ' ');

private:
    bool GetPhrasePinYin(const std::string & phrase, std::string & phrase_pinyin, bool with_tone = false,
                         const char seperator = ' ');
    bool GetCharPinYin(const wchar_t wc, std::vector<std::string> & pinyins, bool with_tone = false,
                       const char seperator = ' ');
    std::string GetFirstLettersOfPinYin(const std::string & pinyin, const char seperator) const;

private:
    static int Callback_Phrase(void * data, int nCols, char ** ppColValue, char ** ppColName);
    static int Callback_HanZi(void * data, int nCols, char ** ppColValue, char ** ppColName);

private:
    sqlite3 * __db;
    std::shared_ptr<cppjieba::Jieba> __jieba;
    Config __config;
};
} // namespace HanZiToPinYin_NS
