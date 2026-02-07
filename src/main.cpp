#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include "silero_vad.h"
#include "audio_capture.h"
#include "whisper_worker.h"
#include "cloud_stt_worker.h"
#include "send_input.h"
#include "mvi_utils.h"
#include <fmt/format.h>
#include <windows.h>

// Configuration
enum class SttProvider
{
    LocalWhisper,
    CloudSiliconFlow
};

SttProvider g_stt_provider = SttProvider::CloudSiliconFlow; // Default to Cloud for testing
std::string g_cloud_token;

int main()
{
    // Set console code page to UTF-8 so console can display UTF-8 characters correctly
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Set up token
    g_cloud_token = mvi_utils::retrive_token();

    printf("--- METASEQUOIA VOICE INPUT START ---\n");
    fflush(stdout);

    try
    {
        printf("[INIT] Loading Silero VAD...\n");
        fflush(stdout);
        SileroVad vad(L"C:\\Users\\sonnycalcr\\EDisk\\CppCodes\\IMECodes\\MetasequoiaVoiceInput\\build\\bin\\Debug\\models\\silero_vad.onnx");
        printf("[INIT] VAD Loaded.\n");
        fflush(stdout);

        std::unique_ptr<SttService> stt;

        if (g_stt_provider == SttProvider::LocalWhisper)
        {
            printf("[INIT] Loading Local Whisper model...\n");
            stt = std::make_unique<WhisperWorker>("C:\\Users\\sonnycalcr\\EDisk\\CppCodes\\IMECodes\\MetasequoiaVoiceInput\\build\\bin\\Debug\\models\\ggml-small.bin");
            printf("[INIT] Local Whisper Loaded.\n");
        }
        else if (g_stt_provider == SttProvider::CloudSiliconFlow)
        {
            printf("[INIT] Initializing Cloud STT (SiliconFlow)...\n");
            stt = std::make_unique<CloudSttWorker>(g_cloud_token);
            printf("[INIT] Cloud STT Ready.\n");
        }
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
                    std::string text = stt->recognize(segment.samples);
                    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                    std::chrono::duration<double> elapsed = end - start;
                    std::cout << "[STT] Time: " << elapsed.count() << " seconds" << std::endl;
                    fflush(stdout);
                    if (!text.empty())
                    {
                        printf("[STT] Recognized: %s\n", text.c_str());
                        fflush(stdout);

                        std::wstring wtext = mvi_utils::utf8_to_wstring(text);
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