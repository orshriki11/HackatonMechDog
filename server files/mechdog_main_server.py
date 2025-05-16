

from flask import Flask, request, jsonify, render_template
import numpy as np
import sounddevice as sd
import threading
import cv2
import openai
import base64
import pyttsx3
import os
import requests
from dotenv import load_dotenv

openai.api_key = "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"


client = openai.OpenAI(api_key="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")
# Load environment variables
load_dotenv()

app = Flask(__name__, static_url_path='/static')

# Audio parameters
SAMPLE_RATE = 16000
CHANNELS = 2
DTYPE = np.int16

IMAGE_CHECK_NOAI = True


# Define ESP32's IP address (get this from ESP32 serial monitor after it connects to Wi-Fi)
# IMPORTANT: REPLACE THIS WITH YOUR ESP32'S ACTUAL IP ADDRESS
ESP32_CAM_IP = "192.168.137.216" # <--- !!! CHANGE THIS TO YOUR ESP32's IP !!!
ESP32_TRIGGER_URL = f"http://{ESP32_CAM_IP}/take_photo" # Endpoint on ESP32




# Ensure the 'static' folder exists for saving images
STATIC_FOLDER = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'static')
if not os.path.exists(STATIC_FOLDER):
    os.makedirs(STATIC_FOLDER)
    print(f"Created static folder at: {STATIC_FOLDER}")



# --- NEW: Flask Route to Trigger ESP32 Photo Capture ---
@app.route('/trigger_photo_capture', methods=['POST'])
def trigger_photo_capture():
    print(f"Attempting to trigger ESP32 at: {ESP32_TRIGGER_URL}")
    try:
        # Send a POST request to the ESP32 to trigger photo capture
        # Use a short timeout so the Flask app doesn't hang if ESP32 is unresponsive
        response = requests.post(ESP32_TRIGGER_URL, timeout=5)
        response.raise_for_status() # Raise an exception for HTTP errors (4xx or 5xx)
        print(f"ESP32 Trigger Response: {response.text}")
        return jsonify({"status": "success", "message": "Photo capture signal sent to ESP32."}), 200
    except requests.exceptions.Timeout:
        print("ESP32 did not respond in time (timeout).")
        return jsonify({"status": "error", "message": "ESP32 did not respond (timeout). It might be offline or busy."}), 500
    except requests.exceptions.ConnectionError:
        print("Could not connect to ESP32. Check IP address or network connectivity.")
        return jsonify({"status": "error", "message": "Could not connect to ESP32. Check IP address or network connectivity."}), 500
    except requests.exceptions.RequestException as e:
        print(f"Error sending trigger to ESP32: {e}")
        return jsonify({"status": "error", "message": f"Failed to send trigger to ESP32: {e}"}), 500
    except Exception as e:
        print(f"An unexpected error occurred during trigger: {e}")
        return jsonify({"status": "error", "message": f"An unexpected error occurred: {e}"}), 500





# --- NEW: Flask Route to Serve the Web Page ---
@app.route('/')
def index():
    """Renders the main web page to display the camera feed."""
    return render_template('index.html')

# --- Existing: Flask Route to Receive Image ---
@app.route('/upload_image', methods=['POST'])
def upload_image():
    """Receives image from the ESP32 robot, saves it, and sends it to OpenAI for analysis."""
    if 'image/jpeg' not in request.content_type:
        print(f"Received unexpected Content-Type: {request.content_type}")
        return jsonify({"status": "error", "message": "Expected Content-Type: image/jpeg"}), 400

    image_data = request.data # Get the raw image data from the request body

    if not image_data:
        return jsonify({"status": "error", "message": "No image data received"}), 400

    print(f"Received image: {len(image_data)} bytes")

    # --- SAVE THE IMAGE TO THE STATIC FOLDER ---
    image_path = os.path.join(STATIC_FOLDER, "latest_image.jpeg")
    try:
        with open(image_path, "wb") as f:
            f.write(image_data)
        print(f"Image saved to {image_path}")
    except Exception as e:
        print(f"Error saving image: {e}")
        # Even if saving fails, try to proceed with OpenAI analysis if image_data is available
        # But for display, saving is crucial.
        return jsonify({"status": "error", "message": f"Failed to save image: {e}"}), 500


    # --- Prepare image for OpenAI Vision API ---
    base64_image = base64.b64encode(image_data).decode('utf-8')
    image_url = f"data:image/jpeg;base64,{base64_image}"

    # --- Call OpenAI Vision API ---
    if(IMAGE_CHECK_NOAI != True):
        try:
            print("Sending image to OpenAI for analysis...")
            response = client.chat.completions.create(
                model="gpt-4o", # Or "gpt-4-vision-preview" if gpt-4o is not yet available to you
                messages=[
                    {
                        "role": "user",
                        "content": [
                            {"type": "text", "text": "Summarize this image in 1-2 sentences. If there is a person injured/in distress far in the image end with FGetCloser"},
                            {
                                "type": "image_url",
                                "image_url": {
                                    "url": image_url,
                                    "detail": "low" # 'low' uses less tokens, 'high' sends more detail for better analysis but costs more.
                                },
                            },
                        ],
                    }
                ],
                max_tokens=300, # Max tokens for the AI's response
            )

            ai_response_content = response.choices[0].message.content
            print(f"OpenAI Response: {ai_response_content}")

            return jsonify({
                "status": "success",
                "message": "Image received and analyzed by OpenAI",
                "analysis": ai_response_content
            }), 200

        except Exception as e:
            print(f"Error calling OpenAI API: {e}")
            return jsonify({"status": "error", "message": f"Failed to analyze image with OpenAI: {e}"}), 500



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
