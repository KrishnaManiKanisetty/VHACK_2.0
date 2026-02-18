#ifndef ASSISTANT_BRAIN_H
#define ASSISTANT_BRAIN_H

#include <string>
#include <vector>

class AssistantBrain {
public:
    AssistantBrain();
    ~AssistantBrain();

    bool init(const std::string& llm_model_path, const std::string& stt_model_path);
    
    std::string processAudio(const std::vector<float>& audio_data);
    std::string generateResponse(const std::string& prompt);

private:
    // Pointers to llama.cpp and whisper.cpp contexts will go here
    void* llama_ctx;
    void* whisper_ctx;
};

#endif // ASSISTANT_BRAIN_H
