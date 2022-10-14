#include <fstream>
#include <pch/pch.h>
#include <sstream>
#include <stdexcept>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/error/en.h>

#include <config/config.h>

using namespace HanZiToPinYin_NS;

void Config::LoadFromFile(const std::string & path)
{
    std::ifstream ifs(path);

    if (ifs.fail())
    {
        std::ostringstream oss;
        oss << "failed to open file (" << path << ')';
        throw std::invalid_argument(oss.str());
    }

    rapidjson::IStreamWrapper isw(ifs);

    rapidjson::Document jd;
    jd.ParseStream(isw);

    if (jd.HasParseError())
    {
        rapidjson::ParseErrorCode e_code = jd.GetParseError();
        const RAPIDJSON_ERROR_CHARTYPE * e_msg = rapidjson::GetParseError_En(e_code);
        throw std::runtime_error(e_msg);
    }

    const auto & jv_db_config = jd["db_config"];

    const rapidjson::Value * jv = nullptr;

    jv = &jv_db_config["sqlite_file_path"];
    __db_config.SetSqliteFilePath(std::string(jv->GetString(), jv->GetStringLength()));

    jv = &jv_db_config["table_name_for_char"];
    __db_config.SetTableNameForChar(std::string(jv->GetString(), jv->GetStringLength()));

    jv = &jv_db_config["table_name_for_phrase"];
    __db_config.SetTableNameForPhrase(std::string(jv->GetString(), jv->GetStringLength()));
}
