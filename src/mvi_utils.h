//
// MetasequoiaVoiceInput Utils
//
#pragma once

#include <string>

// Convert UTF-8 std::string to std::wstring
namespace mvi_utils
{
std::wstring utf8_to_wstring(const std::string &str);
std::string retrive_token();
} // namespace mvi_utils
