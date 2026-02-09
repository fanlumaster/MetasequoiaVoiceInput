#include "mvi_utils.h"
#include <windows.h>
#include <fstream>

// Convert UTF-8 std::string to std::wstring
std::wstring mvi_utils::utf8_to_wstring(const std::string &str)
{
    if (str.empty())
        return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string mvi_utils::retrive_token()
{
    std::string cloud_token;
    char *buf = nullptr;
    size_t sz = 0;
    // Use _dupenv_s instead of getenv to avoid C4996 warning and ensure thread safety
    if (_dupenv_s(&buf, &sz, "LOCALAPPDATA") == 0 && buf != nullptr)
    {
        std::string token_path = std::string(buf) + "\\MetasequoiaVoiceInput\\token.txt";
        std::ifstream token_file(token_path);
        if (token_file.is_open())
        {
            std::getline(token_file, cloud_token);
            token_file.close();
        }
        free(buf);
    }
    return cloud_token;
}

std::wstring mvi_utils::get_vad_model_path()
{
    std::string vad_model_path;
    char *buf = nullptr;
    size_t sz = 0;
    // Use _dupenv_s instead of getenv to avoid C4996 warning and ensure thread safety
    if (_dupenv_s(&buf, &sz, "LOCALAPPDATA") == 0 && buf != nullptr)
    {
        vad_model_path = std::string(buf) + "\\MetasequoiaVoiceInput\\models\\silero_vad.onnx";
        free(buf);
    }
    return utf8_to_wstring(vad_model_path);
}

std::string mvi_utils::get_ggml_model_path()
{
    std::string ggml_model_path;
    char *buf = nullptr;
    size_t sz = 0;
    // Use _dupenv_s instead of getenv to avoid C4996 warning and ensure thread safety
    if (_dupenv_s(&buf, &sz, "LOCALAPPDATA") == 0 && buf != nullptr)
    {
        ggml_model_path = std::string(buf) + "\\MetasequoiaVoiceInput\\models\\ggml-small.bin";
        free(buf);
    }
    return ggml_model_path;
}