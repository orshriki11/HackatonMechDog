from flask import Flask, request, jsonify, send_file, render_template
import numpy as np
import sounddevice as sd
import threading
import cv2
import openai
import base64
import pyttsx3
import os
import urllib.request  # Changed from requests to urllib.request
from dotenv import load_dotenv
import traceback

# Load environment variables
load_dotenv()

app = Flask(__name__, static_url_path='/static')

# Audio parameters
SAMPLE_RATE = 16000
CHANNELS = 2  # Set to 2 for stereo audio
DTYPE = np.int16
OUTPUT_WAV_FILENAME = "received_audio.wav"

# OpenAI API Key
openai.api_key = ""  # Replace with your actual API key

# Constants for OpenAI API limits
MAX_TOKENS = 100  # Maximum tokens in the response
TIMEOUT_SECONDS = 10  # Timeout for the OpenAI API request
TTS_MODEL = "gpt-4o-mini-tts"  # The cheapest TTS model
TTS_VOICE = "alloy"  # Pick a voice
IMAGE_CHECK_NOAI = True

# --- ESP32 IP Addresses ---
# TODO: IMPORTANT: Replace these with your ESP32's actual IP addresses.
# You can get these from the serial monitor of each ESP32 after they connect to Wi-Fi.
ESP32_CAM_IP = "192.168.1.100"  # Example IP for the Camera ESP32 (Replace with actual)
ESP32_ROBOT_IP = "192.168.1.45"  # Example IP for the Robot ESP32 (Replace with actual)
ESP32_MIC_IP = "192.168.137.124" # Example IP for the Microphone ESP32.
ESP32_TRIGGER_URL = f"http://{ESP32_CAM_IP}/take_photo"  # Endpoint on Camera ESP32


# Ensure the 'static' folder exists for saving images
STATIC_FOLDER = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'static')
if not os.path.exists(STATIC_FOLDER):
    os.makedirs(STATIC_FOLDER)
    print(f"Created static folder at: {STATIC_FOLDER}")

latest_frame = None  # Global variable to store the latest frame


# --- Helper Functions ---

def send_robot_command(command):
    """
    Sends a command to the ESP32 robot.  Uses urllib.request
    Args:
        command (str): The command to send (e.g., "move_forward", "turn_left").
    """
    robot_url = f"http://{ESP32_ROBOT_IP}/{command}"  # Construct the URL
    print(f"Sending command '{command}' to robot at {robot_url}")
    try:
        with urllib.request.urlopen(robot_url, timeout=5) as response:
            if response.getcode() == 200:
                print(f"✅ Robot command '{command}' sent successfully!")
            else:
                print(f"❌ Error sending command: {response.getcode()} - {response.read().decode()}")
    except Exception as e:
        print(f"❌ Error sending command '{command}': {e}")



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
    sd.play(stereo, SAMPLE_RATE)
    sd.wait()
    print("✅ Playback done")



def transcribe_audio(audio_file_path):
    """
    Transcribes the audio file using OpenAI's API.  Uses Whisper for audio transcription.

    Args:
        audio_file_path (str): Path to the audio file.

    Returns:
        str: The transcribed text, or None on error.
    """
    try:
        with open(audio_file_path, "rb") as audio_file:
            transcript = openai.audio.transcriptions.create(  # Change here
                model="whisper-1",  # Use the Whisper model for transcription
                file=audio_file
            )
        return transcript.text
    except Exception as e:
        print(f"❌ Error during audio transcription: {e}")
        return None



def get_openai_response(prompt):
    """
    Gets a response from OpenAI's API based on the given prompt, with limits.
    Uses GPT-4.1-mini for chat completion.

    Args:
        prompt (str): The prompt to send to OpenAI.

    Returns:
        str: The response from OpenAI, or None on error.
    """
    try:
        completion = openai.chat.completions.create(  # Change here
            model="gpt-4o-mini",  # Use GPT-4.1-mini for the chat response
            messages=[
                {"role": "user", "content": prompt}
            ],
            max_tokens=MAX_TOKENS,  # Limit the response length
            timeout=TIMEOUT_SECONDS,  # Set a timeout
        )
        return completion.choices[0].message.content
    except openai.error.Timeout as e:
        print(f"❌ OpenAI Timeout Error: {e}")
        return "Error: OpenAI request timed out."
    except openai.error.APIError as e:
        print(f"❌ OpenAI API Error: {e}")
        return "Error: OpenAI API error."
    except Exception as e:
        print(f"❌ Error getting OpenAI response: {e}")
        return None



def generate_speech(text):
    """
    Generates speech from the given text using OpenAI's TTS API.

    Args:
        text (str): The text to convert to speech.

    Returns:
        bytes: The MP3 audio data, or None on error.
    """
    try:
        response = openai.audio.speech.create(
            model=TTS_MODEL,
            voice=TTS_VOICE,
            input=text,
        )
        return response.content
    except Exception as e:
        print(f"❌ Error generating speech: {e}")
        return None



def play_mp3(mp3_data):
    """
    Plays the MP3 audio data on the server's default audio device.

    Args:
        mp3_data (bytes): The MP3 audio data.
    """
    try:
        # Create a file-like object from the MP3 data
        mp3_file = io.BytesIO(mp3_data)
        # Use sounddevice to play the MP3 data
        sd.play(mp3_file)  # Changed here
        sd.wait()
        print("✅ MP3 playback on server done.")
    except Exception as e:
        print(f"❌ Error playing MP3 on server: {e}")



def send_capture_signal():
    """
    Sends a signal to the ESP32 camera to capture an image using urllib.request.
    Runs in a separate thread.
    """
    try:
        with urllib.request.urlopen(ESP32_TRIGGER_URL, timeout=5) as response: # added timeout
            if response.getcode() == 200:
                print("📸 Signal sent to ESP32 Camera successfully!")
            else:
                print(f"❌ Error sending signal to ESP32 Camera: {response.getcode()} - {response.read().decode()}")
    except Exception as e:
        print(f"❌ Error sending signal to ESP32 Camera: {e}")



def analyze_image(image_path):
    """
    Analyzes an image using OpenAI's Vision API.

    Args:
        image_path (str): Path to the image file.

    Returns:
        str: The analysis result from OpenAI, or None on error.
    """
    try:
        with open(image_path, "rb") as img_file:
            base64_image = base64.b64encode(img_file.read()).decode('utf-8')

        prompt = "Describe the situation of the person in this image. Are they injured, distressed, or in need of help? Provide a concise categorization and suggested action."

        response = openai.chat.completions.create(
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
    except Exception as e:
        print(f"❌ Error analyzing image: {e}")
        return None


def speak(text):
    """
    Speaks the given text using pyttsx3.
    """
    engine = pyttsx3.init()
    engine.setProperty('rate', 150)
    engine.say(text)
    engine.runAndWait()



# --- Flask Routes ---

@app.route('/')
def index():
    """Renders the main web page to display the camera feed."""
    return render_template('index.html')



@app.route('/trigger_photo_capture', methods=['POST'])
def trigger_photo_capture():
    """
    Triggers the ESP32 camera to capture an image.
    """
    print(f"Attempting to trigger ESP32 Camera at: {ESP32_TRIGGER_URL}")
    try:
        # Use urllib.request.urlopen instead of requests.post
        with urllib.request.urlopen(ESP32_TRIGGER_URL, timeout=5) as response:  # Add timeout
            if response.getcode() == 200:
                print("📸 Photo capture signal sent to ESP32 Camera successfully!")
                return jsonify({"status": "success", "message": "Photo capture signal sent to ESP32 Camera."}), 200
            else:
                error_message = f"ESP32 Camera returned an error: {response.getcode()} - {response.read().decode()}"
                print(error_message)
                return jsonify({"status": "error", "message": error_message}), 500
    except urllib.error.URLError as e:  # Catch urllib.error.URLError
        error_message = f"Could not connect to ESP32 Camera. Check IP address or network connectivity.  Error: {e}"
        print(error_message)
        return jsonify({"status": "error", "message": error_message}), 500
    except Exception as e:
        error_message = f"An unexpected error occurred during trigger: {e}"
        print(error_message)
        traceback.print_exc()
        return jsonify({"status": "error", "message": error_message}), 500



@app.route('/upload_image', methods=['POST'])
def upload_image():
    """
    Receives image from the ESP32 camera, saves it, and optionally sends it to OpenAI for analysis.
    """
    if 'image/jpeg' not in request.content_type:
        print(f"Received unexpected Content-Type: {request.content_type}")
        return jsonify({"status": "error", "message": "Expected Content-Type: image/jpeg"}), 400

    image_data = request.data  # Get the raw image data from the request body

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
        error_message = f"Error saving image: {e}"
        print(error_message)
        return jsonify({"status": "error", "message": error_message}), 500  # Important: Return error!

    # --- Prepare image for OpenAI Vision API ---
    base64_image = base64.b64encode(image_data).decode('utf-8')
    image_url = f"data:image/jpeg;base64,{base64_image}"

    # --- Call OpenAI Vision API ---
    if not IMAGE_CHECK_NOAI:  # Corrected to use the negation of your flag
        try:
            print("Sending image to OpenAI for analysis...")
            response = openai.chat.completions.create(
                model="gpt-4-vision-preview",  # Or "gpt-4-vision-preview"
                messages=[
                    {
                        "role": "user",
                        "content": [
                            {"type": "text", "text": "Summarize this image in 1-2 sentences. If there is a person injured/in distress, mention that specifically."},
                            {
                                "type": "image_url",
                                "image_url": {
                                    "url": image_url,
                                    "detail": "low"  # 'low' uses less tokens, 'high' sends more detail
                                },
                            },
                        ],
                    }
                ],
                max_tokens=300,  # Max tokens for the AI's response
            )

            ai_response_content = response.choices[0].message.content
            print(f"OpenAI Response: {ai_response_content}")

            return jsonify({
                "status": "success",
                "message": "Image received and analyzed by OpenAI",
                "analysis": ai_response_content
            }), 200

        except Exception as e:
            error_message = f"Error calling OpenAI API: {e}"
            print(error_message)
            return jsonify({"status": "error", "message": error_message}), 500
    else:
        return jsonify({
                "status": "success",
                "message": "Image received, but OpenAI analysis is skipped (IMAGE_CHECK_NOAI is True)",
            }), 200



@app.route("/upload_audio", methods=["POST"])
def upload_audio():
    """
    Handles audio upload, saves it, transcribes it, and gets an OpenAI response.
    """
    if request.method == "POST":
        audio_data = request.get_data()
        print(f"📥 Received {len(audio_data)} bytes of audio")

        # Check if audio data is empty
        if not audio_data:
            return jsonify({"status": "error", "message": "No audio data received"}), 400

        # Save the audio to a WAV file
        try:
            write_wav(OUTPUT_WAV_FILENAME, SAMPLE_RATE, np.frombuffer(audio_data, dtype=DTYPE))
            print(f"💾 Audio saved as '{OUTPUT_WAV_FILENAME}'")
        except Exception as e:
            error_message = f"Error saving audio: {e}"
            print(error_message)
            traceback.print_exc()
            return jsonify({"status": "error", "message": error_message}), 500

        # Play audio in a separate thread
        threading.Thread(target=play_audio, args=(audio_data,), daemon=True).start()

        # Transcribe the audio
        transcription = transcribe_audio(OUTPUT_WAV_FILENAME)
        if not transcription:
            return jsonify({"status": "error", "message": "Audio transcription failed"}), 500

        print(f"📝 Transcribed text: {transcription}")

        # Check for "help" and send signal to ESP32 Robot
        if "help" in transcription.lower():
            print("🚨 \'Help\' detected! Sending signal to ESP32 Robot...")
            threading.Thread(target=send_robot_command, args=("move_forward",)).start()  # Example command
            threading.Thread(target=send_capture_signal).start() # also take a picture

        # Get the OpenAI response
        openai_prompt = f"The following is the transcription of a voice recording: {transcription}.  Please provide a concise and informative answer."
        openai_response = get_openai_response(openai_prompt)
        if not openai_response:
            return jsonify({"status": "error", "message": "Failed to get response from OpenAI"}), 500

        print(f"🤖 OpenAI Response: {openai_response}")

        # Generate speech from the OpenAI response
        speech_data = generate_speech(openai_response)  # this is in wav
        # save the speech data to answer WAV file

        try:
            with open("answer.wav", "wb") as f:
                f.write(speech_data)
            print(f"💾 Answer audio saved as \'answer.wav\'")
        except Exception as e:
            error_message = f"❌ Error saving answer audio to WAV: {e}"
            print(error_message)
            return jsonify({
                "status": "error",
                "message": "Speech generation failed: Error saving answer audio"
            }), 500

        if speech_data:
            print("🗣️  Speech generated successfully")
            # convert to mp3
            try:
                from pydub import AudioSegment  # moved here to avoid issues if pydub is not installed
                from pydub.utils import make_chunks
                audio_segment = AudioSegment.from_wav(io.BytesIO(speech_data))
                mp3_bio = io.BytesIO()
                audio_segment.export(mp3_bio, format="mp3")
                mp3_data = mp3_bio.getvalue()

                # Play the MP3 data on the server's speakers in a separate thread
                threading.Thread(target=play_mp3, args=(mp3_data,), daemon=True).start()  # changed here

                # Delete the audio file after processing
                try:
                    os.remove(OUTPUT_WAV_FILENAME)
                    print(f"🗑️ Deleted audio file: {os.path.abspath(OUTPUT_WAV_FILENAME)}")
                except Exception as e:
                    print(f"❌ Error deleting audio file: {e}")
                # Return the MP3 data as a file attachment
                return send_file(io.BytesIO(mp3_data), mimetype="audio/mp3", as_attachment=True,
                                 download_name="response.mp3")
            except Exception as e:
                print(f"❌ Error converting to MP3: {e}")
                # Return the WAV data instead of the MP3 data
                print("Returning WAV audio data instead.")
                return send_file(io.BytesIO(speech_data), mimetype="audio/wav", as_attachment=True,
                                 download_name="response.wav")
        else:
            return jsonify({
                "status": "success",
                "bytes_received": len(audio_data),
                "transcription": transcription,
                "openai_response": openai_response,
                "message": "Speech generation failed",
            }), 200  # Still success because other operations were successful



# @app.route("/upload", methods=["POST"])
# def upload():
#     """
#     Handles image upload from the ESP32.  This is for the *video stream*, not the single image analysis.
#     """
#     global latest_frame
#     img_bytes = request.data
#     np_arr = np.frombuffer(img_bytes, np.uint8)
#     frame = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)
#     latest_frame = frame
#     return 'OK', 200



@app.route('/analyze', methods=['POST'])
def analyze():
    """
    Analyzes a single image sent from a client (not the ESP32 camera).
    """
    if 'image' not in request.files:
        return jsonify({"error": "No image provided"}), 400

    image = request.files['image']
    image_path = "temp.jpg"  # Use a temporary file
    image.save(image_path)

    result = analyze_image(image_path)  # Use the analyze_image function

    if request.form.get('speak') == 'yes':
        speak(result)

    os.remove(image_path)  # Delete the temporary file
    return jsonify({"analysis": result})



if __name__ == "__main__":
    print("🎧 Starting server on http://0.0.0.0:12345")
    app.run(host="0.0.0.0", port=12345, threaded=True)  # Enable threading for concurrent requests if needed.