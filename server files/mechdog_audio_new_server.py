# this is an old server code

from flask import Flask, request, jsonify, Response
import numpy as np
import sounddevice as sd
import threading
import wave
from datetime import datetime
from wave_saver import save_as_wav

app = Flask(__name__)

# ───────────────────────────────────────────────────────────
# 1) CONFIGURATION
# ───────────────────────────────────────────────────────────

WAV_PATH = "Kalimba.wav"  # Your WAV file (any sample rate / channels)

# Read WAV parameters and raw PCM
with wave.open(WAV_PATH, "rb") as wf:
    SAMPLE_RATE = wf.getframerate()
    CHANNELS    = wf.getnchannels()
    WIDTH       = wf.getsampwidth()          # bytes per sample
    n_frames    = wf.getnframes()
    raw_pcm     = wf.readframes(n_frames)

PCM_LEN = len(raw_pcm)
DTYPE   = {1: np.int8, 2: np.int16, 4: np.int32}[WIDTH]

print(f"Loaded '{WAV_PATH}':")
print(f"  • Sample Rate: {SAMPLE_RATE} Hz")
print(f"  • Channels:    {CHANNELS}")
print(f"  • Width:       {WIDTH*8} bits")
print(f"  • PCM bytes:   {PCM_LEN}")

# ───────────────────────────────────────────────────────────
# 2) PLAYBACK RECEIVED AUDIO
# ───────────────────────────────────────────────────────────

def play_received(data_bytes: bytes):
    arr = np.frombuffer(data_bytes, dtype=DTYPE)
    try:
        frames = arr.reshape(-1, CHANNELS)
    except Exception:
        print("❌ Received data length not a multiple of frame size")
        return

    print("▶️ Playing ESP32 audio...")
    sd.play(frames, samplerate=SAMPLE_RATE)
    sd.wait()
    print("✅ ESP32 audio done")

# ───────────────────────────────────────────────────────────
# 3) UPLOAD ENDPOINT
# ───────────────────────────────────────────────────────────
@app.route('/upload', methods=['POST'])
def upload():
    global audio_data
    audio_data = request.data
    print("✅ ESP32 audio done")
    print(f"📥 /upload got {len(audio_data)} bytes")

    # ✅ Save to file for debugging
    # timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    # with open(f"temp_audio/recorded_{timestamp}.raw", "wb") as f:
    #     f.write(audio_data)
    # save_as_wav(audio_data, f"recorded_{timestamp}.wav")

    return "OK"

# ───────────────────────────────────────────────────────────
# 4) DOWNLOAD /sound ENDPOINT
# ───────────────────────────────────────────────────────────

@app.route("/sound", methods=["GET"])
def serve_sound():
    headers = {
        "Content-Type": "application/octet-stream",
        "Content-Length": str(PCM_LEN),
        "X-Sample-Rate": str(SAMPLE_RATE),
        "X-Channels": str(CHANNELS),
        "X-Width": str(WIDTH)
    }
    return Response(raw_pcm, headers=headers, status=200)

# ───────────────────────────────────────────────────────────
# 5) RUN SERVER
# ───────────────────────────────────────────────────────────

if __name__ == "__main__":
    print("🟢 Server endpoints:")
    print("   POST /upload  → play uploaded audio")
    print("   GET  /sound   → download Kalimba raw PCM")
    app.run(host="0.0.0.0", port=12345)