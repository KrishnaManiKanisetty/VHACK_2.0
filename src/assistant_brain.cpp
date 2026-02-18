#include "assistant_brain.h"
#include "../lib/llama.cpp/include/llama.h"
#include "../lib/whisper.cpp/include/whisper.h"
#include <iostream>
#include <vector>

AssistantBrain::AssistantBrain() : llama_ctx(nullptr), whisper_ctx(nullptr) {}

AssistantBrain::~AssistantBrain() {
    if (llama_ctx) llama_free((llama_context*)llama_ctx);
    if (whisper_ctx) whisper_free((whisper_context*)whisper_ctx);
}

bool AssistantBrain::init(const std::string& llm_model_path, const std::string& stt_model_path) {
    // 1. Initialize Whisper
    whisper_context_params wparams = whisper_context_default_params();
    whisper_ctx = whisper_init_from_file_with_params(stt_model_path.c_str(), wparams);
    if (!whisper_ctx) {
        std::cerr << "Failed to load Whisper model: " << stt_model_path << std::endl;
        return false;
    }

    // 2. Initialize Llama
    llama_backend_init();
    llama_model_params mparams = llama_model_default_params();
    llama_model* model = llama_model_load_from_file(llm_model_path.c_str(), mparams);
    if (!model) {
        std::cerr << "Failed to load Llama model: " << llm_model_path << std::endl;
        return false;
    }

    llama_context_params cparams = llama_context_default_params();
    cparams.n_ctx = 2048; // Adjust based on model
    llama_ctx = llama_init_from_model(model, cparams);
    if (!llama_ctx) {
        std::cerr << "Failed to create Llama context" << std::endl;
        return false;
    }

    return true;
}

std::string AssistantBrain::processAudio(const std::vector<float>& audio_data) {
    if (!whisper_ctx) return "";

    whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    wparams.n_threads = 4;
    wparams.print_progress = false;

    if (whisper_full((whisper_context*)whisper_ctx, wparams, audio_data.data(), audio_data.size()) != 0) {
        return "";
    }

    std::string result = "";
    int n_segments = whisper_full_n_segments((whisper_context*)whisper_ctx);
    for (int i = 0; i < n_segments; ++i) {
        result += whisper_full_get_segment_text((whisper_context*)whisper_ctx, i);
    }
    return result;
}

std::string AssistantBrain::generateResponse(const std::string& prompt) {
    if (!llama_ctx) return "Brain not initialized.";

    llama_context* ctx = (llama_context*)llama_ctx;
    const llama_model* model = llama_get_model(ctx);

    // 1. Tokenize prompt
    std::vector<llama_token> tokens(prompt.size() + 2);
    int n_tokens = llama_tokenize(llama_model_get_vocab(model), prompt.c_str(), prompt.size(), tokens.data(), tokens.size(), true, true);
    tokens.resize(n_tokens);

    // 2. Clear KV cache
    llama_memory_clear(llama_get_memory(ctx), true);

    // 3. Simple inference loop
    std::string response = "";
    llama_batch batch = llama_batch_init(512, 0, 1);

    for (int i = 0; i < tokens.size(); ++i) {
        llama_batch_add(batch, tokens[i], i, { 0 }, i == tokens.size() - 1);
    }

    if (llama_decode(ctx, batch) != 0) return "Decode failed.";

    for (int i = 0; i < 64; ++i) { // Limit response length
        auto* samplers = llama_sampler_chain_init(llama_sampler_chain_default_params());
        llama_sampler_chain_add(samplers, llama_sampler_init_temp(0.7f));
        llama_sampler_chain_add(samplers, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

        llama_token new_token = llama_sampler_sample(samplers, ctx, -1);
        llama_sampler_free(samplers);

        if (llama_vocab_is_eog(llama_model_get_vocab(model), new_token)) break;

        char buf[128];
        int n = llama_token_to_piece(llama_model_get_vocab(model), new_token, buf, sizeof(buf), 0, true);
        if (n > 0) response.append(buf, n);

        llama_batch_clear(batch);
        llama_batch_add(batch, new_token, tokens.size() + i, { 0 }, true);
        if (llama_decode(ctx, batch) != 0) break;
    }

    llama_batch_free(batch);
    return response;
}
