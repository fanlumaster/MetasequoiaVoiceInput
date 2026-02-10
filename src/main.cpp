#include <iostream>
#include <string>
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
    // g_stt_provider = SttProvider::LocalWhisper;

    // Set console code page to UTF-8 so console can display UTF-8 characters correctly
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Set up token
    g_cloud_token = mvi_utils::retrive_token();

    // silero_vad.onnx path
    std::wstring vad_model_path = mvi_utils::get_vad_model_path();

    // ggml model path
    std::string ggml_model_path = mvi_utils::get_ggml_model_path();

    printf("--- METASEQUOIA VOICE INPUT START ---\n");
    fflush(stdout);

    try
    {
        printf("[INIT] Loading Silero VAD...\n");
        fflush(stdout);
        SileroVad vad(vad_model_path);
        printf("[INIT] VAD Loaded.\n");
        fflush(stdout);

        std::unique_ptr<SttService> stt;

        if (g_stt_provider == SttProvider::LocalWhisper)
        {
            printf("[INIT] Loading Local Whisper model...\n");
            stt = std::make_unique<WhisperWorker>(ggml_model_path.c_str());
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
        //
        // STT queue + thread, 把耗时的操作放在这个线程里，避免在 audio callback 里做耗时操作导致丢音频
        //
        std::mutex stt_mutex;
        std::condition_variable stt_cv;
        std::deque<SpeechSegment> stt_queue;
        std::atomic<bool> stt_stop = false;
        std::thread stt_thread([&]() {
            while (!stt_stop)
            {
                SpeechSegment seg;

                {
                    std::unique_lock<std::mutex> lock(stt_mutex);
                    stt_cv.wait(lock, [&]() { return stt_stop || !stt_queue.empty(); });

                    if (stt_stop)
                        break;

                    seg = std::move(stt_queue.front());
                    stt_queue.pop_front();
                }

                // 只有这里才允许慢操作
                auto start = std::chrono::steady_clock::now();
                std::string text = stt->recognize(seg.samples);
                auto end = std::chrono::steady_clock::now();
                std::cout << "[STT] Time: " << std::chrono::duration<double>(end - start).count() << "s\n";
                if (!text.empty())
                {
                    printf("[STT] Recognized: %s\n", text.c_str());
                    fflush(stdout);
                    send_text(mvi_utils::utf8_to_wstring(text));
                }
            }
        });

        audio.start([&](const float *data, size_t count) {
            try
            {
                vad.push_audio(data, count);
                while (vad.has_segment())
                {
                    auto segment = vad.pop_segment();
                    {
                        std::lock_guard<std::mutex> lock(stt_mutex);
                        stt_queue.push_back(std::move(segment));
                    }
                    stt_cv.notify_one();
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
        // 通知 stt 线程停止
        stt_stop = true;
        stt_cv.notify_one();
        if (stt_thread.joinable())
        {
            stt_thread.join();
        }
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