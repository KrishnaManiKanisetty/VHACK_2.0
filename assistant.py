import os
import wave
import numpy as np
import sounddevice as sd
from scipy.io import wavfile
import whisper
import pyttsx3
from llama_cpp import Llama

# Configuration
LLM_MODEL_PATH = "models/phi-3-mini-4k-instruct.gguf"
STT_MODEL_NAME = "base.en"
SAMPLE_RATE = 16000
CHANNELS = 1

class VoiceAssistant:
    def __init__(self):
        print("Initializing Voice Assistant...")
        
        # Initialize TTS
        self.tts_engine = pyttsx3.init()
        self.tts_engine.setProperty('rate', 150)
        
        # Initialize STT
        print(f"Loading Whisper model: {STT_MODEL_NAME}...")
        self.stt_model = whisper.load_model(STT_MODEL_NAME)
        
        # Initialize LLM
        print(f"Loading LLM model: {LLM_MODEL_PATH}...")
        self.llm = Llama(model_path=LLM_MODEL_PATH, n_ctx=2048, verbose=False)
        
    def speak(self, text):
        print(f"Assistant: {text}")
        self.tts_engine.say(text)
        self.tts_engine.runAndWait()

    def listen(self, duration=5):
        print(f"\nListening for {duration} seconds...")
        # Record audio
        recording = sd.rec(int(duration * SAMPLE_RATE), samplerate=SAMPLE_RATE, channels=CHANNELS, dtype='int16')
        sd.wait() # Wait until recording is finished
        
        print("Processing audio...")
        temp_filename = "temp_audio.wav"
        wavfile.write(temp_filename, SAMPLE_RATE, recording)
        
        return temp_filename

    def transcribe(self, audio_file):
        result = self.stt_model.transcribe(audio_file)
        if os.path.exists(audio_file):
            os.remove(audio_file)
        return result["text"].strip()

    def generate_response(self, text):
        prompt = f"<|user|>\n{text}<|end|>\n<|assistant|>"
        output = self.llm(prompt, max_tokens=256, stop=["<|end|>"], echo=False)
        return output["choices"][0]["text"].strip()

    def run(self):
        self.speak("Hello! I am your Python voice assistant. How can I help you?")
        
        while True:
            try:
                audio_file = self.listen()
                transcript = self.transcribe(audio_file)
                
                if not transcript or len(transcript) < 2:
                    print("No clear speech detected.")
                    continue
                    
                print(f"You: {transcript}")
                
                if any(word in transcript.lower() for word in ["exit", "stop", "goodbye"]):
                    self.speak("Goodbye!")
                    break
                
                response = self.generate_response(transcript)
                self.speak(response)
                
            except KeyboardInterrupt:
                break
            except Exception as e:
                print(f"Error: {e}")
                break

if __name__ == "__main__":
    assistant = VoiceAssistant()
    assistant.run()
