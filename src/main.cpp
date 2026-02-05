#include "audio_capture.h"
#include "vad.h"
#include "whisper_worker.h"
#include "send_input.h"

#include <windows.h>
#include <codecvt>
#include <locale>

static std::wstring utf8_to_wstring(const std::string &s)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(s);
}

int main()
{
    AudioCapture audio;
    VadSegmenter vad;
    WhisperWorker whisper("C:\\Users\\sonnycalcr\\EDisk\\CppCodes\\IMECodes\\MetasequoiaVoiceInput\\build\\bin\\Debug\\models\\ggml-large-v3.bin");

    audio.start([&](const float *data, size_t count) {
        vad.process(data, count);

        if (vad.should_flush())
        {
            auto pcm = vad.take_audio();
            auto text = whisper.recognize(pcm);
            if (!text.empty())
            {
                send_text(utf8_to_wstring(text));
            }
        }
    });

    MessageBoxA(nullptr, "Voice input running.\nClose to exit.", "voice", MB_OK);
    audio.stop();
}