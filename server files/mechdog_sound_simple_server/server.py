from flask import Flask, request
from datetime import datetime
import os
import wave

app = Flask(__name__)
UPLOAD_FOLDER = "uploads"
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

@app.route('/upload', methods=['POST'])
def upload():
    raw_data = request.data
    num_bytes = len(raw_data)
    print(f"[INFO] Received {num_bytes} bytes")

    # Save raw PCM data
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    raw_filename = os.path.join(UPLOAD_FOLDER, f"{timestamp}.raw")
    with open(raw_filename, "wb") as f:
        f.write(raw_data)

    # Convert to WAV
    wav_filename = os.path.join(UPLOAD_FOLDER, f"{timestamp}.wav")
    with wave.open(wav_filename, 'wb') as wav_file:
        wav_file.setnchannels(1)            # Mono
        wav_file.setsampwidth(2)            # 16-bit
        wav_file.setframerate(16000)        # 16 kHz
        wav_file.writeframes(raw_data)

    print(f"[INFO] WAV saved as {wav_filename}")
    return "OK", 200

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5000)