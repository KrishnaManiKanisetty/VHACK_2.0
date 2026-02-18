#define MINIAUDIO_IMPLEMENTATION
#include "../lib/miniaudio.h"
#include "audio_engine.h"
#include <sapi.h>
#include <sphelper.h>
#include <iostream>

AudioEngine::AudioEngine() : is_recording(false) {}

AudioEngine::~AudioEngine() {
    ma_device_uninit(&device);
}

void AudioEngine::data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    AudioEngine* pEngine = (AudioEngine*)pDevice->pUserData;
    if (pEngine->is_recording && pInput != NULL) {
        float* pSamples = (float*)pInput;
        pEngine->audio_buffer.insert(pEngine->audio_buffer.end(), pSamples, pSamples + frameCount);
    }
}

bool AudioEngine::init() {
    ma_device_config config = ma_device_config_init(ma_device_type_capture);
    config.capture.format = ma_format_f32;
    config.capture.channels = 1;
    config.sampleRate = 16000; // Whisper expects 16kHz
    config.dataCallback = data_callback;
    config.pUserData = this;

    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        return false;
    }
    return true;
}

void AudioEngine::startRecording() {
    audio_buffer.clear();
    is_recording = true;
    ma_device_start(&device);
}

void AudioEngine::stopRecording() {
    is_recording = false;
    ma_device_stop(&device);
}

std::vector<float> AudioEngine::getRecordedAudio() {
    return audio_buffer;
}

void AudioEngine::speak(const std::string& text) {
    // Windows SAPI implementation
    ISpVoice* pVoice = NULL;
    if (FAILED(::CoInitialize(NULL))) return;

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
    if (SUCCEEDED(hr)) {
        // Convert std::string to std::wstring
        int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
        std::wstring wtext(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wtext[0], len);

        pVoice->Speak(wtext.c_str(), 0, NULL);
        pVoice->Release();
        pVoice = NULL;
    }
    ::CoUninitialize();
}
