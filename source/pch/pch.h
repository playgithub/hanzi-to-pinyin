#pragma once

#pragma execution_character_set("utf-8")

#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

// STL
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <regex>
#include <map>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

// SQLite
#include <sqlite3.h>

// rapidjson
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/error/en.h>

// cppjieba
//#include <cppjieba/Jieba.hpp>
