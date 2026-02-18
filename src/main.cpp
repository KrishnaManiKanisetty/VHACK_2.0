#include <iostream>
#include <string>
#include <vector>
#include "audio_engine.h"
#include "assistant_brain.h"

int main() {
    std::cout << "Starting Laptop Voice Assistant..." << std::endl;

    AudioEngine audio;
    AssistantBrain brain;

    if (!audio.init()) {
        std::cerr << "Failed to initialize Audio Engine" << std::endl;
        return 1;
    }

    if (!brain.init("models/phi-3-mini-4k-instruct.gguf", "models/ggml-base.en.bin")) {
        std::cerr << "Failed to initialize Assistant Brain" << std::endl;
        return 1;
    }

    std::cout << "Assistant is ready! Say 'Hello' or something..." << std::endl;

    while (true) {
        std::cout << "\nListening... (Press Enter to start/stop if manual, or automatic voice detection)" << std::endl;
        
        audio.startRecording();
        std::cin.get(); // Simple trigger for now
        audio.stopRecording();

        std::cout << "Processing..." << std::endl;
        std::vector<float> audio_data = audio.getRecordedAudio();
        
        std::string transcript = brain.processAudio(audio_data);
        std::cout << "You said: " << transcript << std::endl;

        if (transcript.empty()) continue;

        std::string response = brain.generateResponse(transcript);
        std::cout << "Assistant: " << response << std::endl;

        audio.speak(response);
    }

    return 0;
}
