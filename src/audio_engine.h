#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include "../lib/miniaudio.h"
#include <vector>
#include <string>
#include <functional>

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool init();
    void startRecording();
    void stopRecording();
    std::vector<float> getRecordedAudio();

    void speak(const std::string& text);

private:
    ma_device device;
    bool is_recording;
    std::vector<float> audio_buffer;
    
    static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
};

#endif // AUDIO_ENGINE_H
