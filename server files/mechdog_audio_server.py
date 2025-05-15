from flask import Flask, request, jsonify
import numpy as np
import sounddevice as sd
import threading
from scipy.io.wavfile import write as write_wav  # Import write function from scipy.io.wavfile
import os  # For managing file paths

#define SERVER_URL      "http://192.168.137.1:12345/upload"

app = Flask(__name__)

# Audio parameters
SAMPLE_RATE = 16000
CHANNELS = 1  # Assuming stereo audio based on your current code's reshape logic
DTYPE = np.int16

# Define a path to save the received WAV file
OUTPUT_WAV_FILENAME = "received_audio.wav"


def play_and_save_audio(data_bytes):
    audio_data = np.frombuffer(data_bytes, dtype=DTYPE)
    processed_audio_data = audio_data # No reshaping needed for mono

    # Save the audio to a WAV file
    try:
        write_wav(OUTPUT_WAV_FILENAME, SAMPLE_RATE, processed_audio_data)
        print(f"💾 Audio saved as '{OUTPUT_WAV_FILENAME}'")
    except Exception as e:
        print(f"❌ Error saving WAV file: {e}")

    # Play audio
    print("▶️ Playing audio...")
    try:
        sd.play(processed_audio_data, SAMPLE_RATE)
        sd.wait()
        print("✅ Playback done")
    except Exception as e:
        print(f"❌ Error during audio playback: {e}")


@app.route("/upload", methods=["POST"])
def upload_audio():
    if request.method == "POST":
        audio_data = request.get_data()
        print(f"📥 Received {len(audio_data)} bytes")

        # Start playback and saving in a separate thread so we don't block HTTP response
        threading.Thread(target=play_and_save_audio, args=(audio_data,), daemon=True).start()

        return jsonify({"status": "success", "bytes_received": len(audio_data), "saved_as": OUTPUT_WAV_FILENAME}), 200


if __name__ == "__main__":
    print("🎧 Starting server on http://0.0.0.0:12345/upload")
    print(f"🚀 Received audio will be saved to '{os.path.abspath(OUTPUT_WAV_FILENAME)}'")
    app.run(host="0.0.0.0", port=12345)