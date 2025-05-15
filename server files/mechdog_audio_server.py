# from flask import Flask, request, Response
# import sounddevice as sd
# import numpy as np

# app = Flask(__name__)

# # Audio parameters (must match ESP32)
# SAMPLE_RATE = 16000
# CHANNELS = 1
# DTYPE = 'int16'  # 16-bit audio

# @app.route('/upload', methods=['POST'])
# def upload_audio():
#     try:
#         raw_audio = request.data  # raw bytes sent from ESP32
        
#         # Convert bytes to numpy array
#         audio_data = np.frombuffer(raw_audio, dtype=DTYPE)

#         # Play audio data asynchronously
#         sd.play(audio_data, samplerate=SAMPLE_RATE)
        
#         # Optional: wait for playback to finish before responding
#         sd.wait()

#         return Response("Audio received and played", status=200)
#     except Exception as e:
#         print(f"Error processing audio: {e}")
#         return Response(f"Error: {e}", status=500)

# if __name__ == '__main__':
#     print("🟢 HTTP server running at http://0.0.0.0:12345/upload")
#     app.run(host='0.0.0.0', port=12345)


from flask import Flask, request, jsonify
import numpy as np
import sounddevice as sd
import threading

app = Flask(__name__)

# Audio parameters
SAMPLE_RATE = 16000
CHANNELS = 2
DTYPE = np.int16

def play_audio(data_bytes):
    """Play received audio using sounddevice."""
    # Convert to numpy array
    audio_data = np.frombuffer(data_bytes, dtype=DTYPE)
    
    # Reshape to stereo (2 channels)
    try:
        stereo = audio_data.reshape(-1, CHANNELS)
    except ValueError:
        print("❌ Invalid audio length for stereo playback")
        return

    print("▶️ Playing audio...")
    # sd.play(stereo, SAMPLE_RATE)
    stereo = audio_data.reshape(-1, CHANNELS)

    # Uncomment one to test individual channels:
    left = stereo[:, 0]
    right = stereo[:, 1]
    # sd.play(left, SAMPLE_RATE)      # Play only left
    sd.play(right, SAMPLE_RATE)     # Play only right
    sd.wait()

    # Default: play both
    # sd.play(stereo, SAMPLE_RATE)
    sd.wait()
    print("✅ Playback done")

@app.route("/upload", methods=["POST"])
def upload_audio():
    if request.method == "POST":
        audio_data = request.get_data()
        print(f"📥 Received {len(audio_data)} bytes")

        # Playback in separate thread so we don't block HTTP response
        threading.Thread(target=play_audio, args=(audio_data,), daemon=True).start()

        return jsonify({"status": "success", "bytes_received": len(audio_data)}), 200

if __name__ == "__main__":
    print("🎧 Starting server on http://0.0.0.0:12345/upload")
    app.run(host="0.0.0.0", port=12345)
