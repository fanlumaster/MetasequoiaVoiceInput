#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include "silero_vad.h"
#include "audio_capture.h"
#include "whisper_worker.h"
#include "send_input.h"
#include <fmt/format.h>
#include <windows.h>

// Convert UTF-8 std::string to std::wstring
std::wstring utf8_to_wstring(const std::string &str)
{
    if (str.empty())
        return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

int main()
{
    // 禁用 stdout
    freopen("nul", "w", stdout);
    freopen("nul", "w", stderr);
    printf("--- METASEQUOIA VOICE INPUT START ---\n");
    fflush(stdout);

    try
    {
        printf("[INIT] Loading Silero VAD...\n");
        fflush(stdout);
        SileroVad vad(L"C:\\Users\\sonnycalcr\\EDisk\\CppCodes\\IMECodes\\MetasequoiaVoiceInput\\build\\bin\\Debug\\models\\silero_vad.onnx");
        printf("[INIT] VAD Loaded.\n");
        fflush(stdout);

        printf("[INIT] Loading Whisper model...\n");
        fflush(stdout);
        WhisperWorker whisper("C:\\Users\\sonnycalcr\\EDisk\\CppCodes\\IMECodes\\MetasequoiaVoiceInput\\build\\bin\\Debug\\models\\ggml-small.bin");
        printf("[INIT] Whisper Loaded.\n");
        fflush(stdout);

        printf("[INIT] Initializing Audio...\n");
        fflush(stdout);
        AudioCapture audio;
        audio.start([&](const float *data, size_t count) {
            try
            {
                vad.push_audio(data, count);
                while (vad.has_segment())
                {
                    auto segment = vad.pop_segment();
                    printf("[VAD] Main thread popping segment (%zu samples)\n", segment.samples.size());
                    fflush(stdout);

                    // auto text = whisper.recognize(segment.samples);
                    std::string text = "test";
                    if (!text.empty())
                    {
                        printf("[WHISPER] Recognized: %s\n", text.c_str());
                        printf("[WHISPER] Raw Bytes: ");
                        for (unsigned char c : text)
                            printf("%02X ", c);
                        printf("\n");
                        fflush(stdout);

                        std::wstring wtext = utf8_to_wstring(text);
                        printf("[MAIN] Converted WString (%zu chars): ", wtext.size());
                        for (wchar_t wc : wtext)
                            printf("%04X ", (unsigned int)wc);
                        printf("\n");
                        fflush(stdout);

                        send_text(wtext);
                    }
                }
            }
            catch (const std::exception &e)
            {
                printf("[CALLBACK ERROR] %s\n", e.what());
                fflush(stdout);
            }
            catch (...)
            {
                printf("[CALLBACK ERROR] Unknown exception\n");
                fflush(stdout);
            }
        });

        printf("Application is running. Press ENTER to stop and exit.\n");
        fflush(stdout);
        std::cin.get();

        printf("Stopping...\n");
        fflush(stdout);
        audio.stop();
        printf("Stopped.\n");
        fflush(stdout);
    }
    catch (const std::exception &e)
    {
        printf("FATAL ERROR: %s\n", e.what());
        fflush(stdout);
        MessageBoxA(nullptr, e.what(), "MetasequoiaVoiceInput - Fatal Error", MB_ICONERROR);
        return 1;
    }

    return 0;
}