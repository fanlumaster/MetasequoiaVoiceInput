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
    // freopen("nul", "w", stdout);
    // freopen("nul", "w", stderr);
    // Set console code page to UTF-8 so console can display UTF-8 characters correctly
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

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

                    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
                    auto text = whisper.recognize(segment.samples);
                    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                    std::chrono::duration<double> elapsed = end - start;
                    std::cout << "[WHISPER] Time: " << elapsed.count() << " seconds" << std::endl;
                    fflush(stdout);
                    if (!text.empty())
                    {
                        printf("[WHISPER] Recognized: %s\n", text.c_str());
                        fflush(stdout);

                        std::wstring wtext = utf8_to_wstring(text);
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