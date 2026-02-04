#pragma once
#include <string>
#include <vector>

struct whisper_context;

class WhisperWorker
{
  public:
    explicit WhisperWorker(const char *model_path);
    ~WhisperWorker();

    std::string recognize(const std::vector<float> &pcm);

  private:
    whisper_context *ctx_;
};