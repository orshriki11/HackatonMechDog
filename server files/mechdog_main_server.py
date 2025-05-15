

from flask import Flask, request, jsonify
import numpy as np
import sounddevice as sd
import threading
import cv2
import openai
import base64
import pyttsx3
import os

openai.api_key = "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"


app = Flask(__name__)

# Audio parameters
SAMPLE_RATE = 16000
CHANNELS = 2
DTYPE = np.int16


def analyze_image(image_path):
    with open(image_path, "rb") as img_file:
        base64_image = base64.b64encode(img_file.read()).decode('utf-8')

    prompt = "Describe the situation of the person in this image. Are they injured, distressed, or in need of help? Provide a concise categorization and suggested action."

    response = openai.ChatCompletion.create(
        model="gpt-4-vision-preview",
        messages=[
            {"role": "system", "content": "You are an emergency assistant robot."},
            {
                "role": "user",
                "content": [
                    {"type": "text", "text": prompt},
                    {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{base64_image}"}}
                ],
            }
        ],
        max_tokens=500
    )
    return response.choices[0].message.content

def speak(text):
    engine = pyttsx3.init()
    engine.setProperty('rate', 150)
    engine.say(text)
    engine.runAndWait()

@app.route('/analyze', methods=['POST'])
def analyze():
    if 'image' not in request.files:
        return jsonify({"error": "No image provided"}), 400

    image = request.files['image']
    image_path = "temp.jpg"
    image.save(image_path)

    result = analyze_image(image_path)

    if request.form.get('speak') == 'yes':
        speak(result)

    os.remove(image_path)
    return jsonify({"analysis": result})

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
def upload():
    global latest_frame
    img_bytes = request.data
    np_arr = np.frombuffer(img_bytes, np.uint8)
    frame = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)
    latest_frame = frame
    return 'OK', 200
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
