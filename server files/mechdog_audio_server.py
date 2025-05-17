# Audio server that receives audio data, transcribes it, generates a response using OpenAI's API, and plays the audio.
# The code was created for the MechDog project part of the AIoT Hackathon 2025


from flask import Flask, request, jsonify, send_file
import numpy as np
import sounddevice as sd
import threading
from scipy.io.wavfile import write as write_wav
import os
import openai  # Import the OpenAI library
import time
from io import BytesIO  # Import BytesIO for handling in-memory files
import io  # Import the io module
import traceback  # Import traceback
import urllib.request # added urllib.request


app = Flask(__name__)

# Audio parameters
SAMPLE_RATE = 16000
CHANNELS = 1
DTYPE = np.int16
OUTPUT_WAV_FILENAME = "received_audio.wav"

# OpenAI API Key
# Make sure to set your OpenAI API key in the environment variable
OPENAI_API_KEY = ""# Set your OpenAI API key here
openai.api_key = OPENAI_API_KEY

# Constants for OpenAI API limits
MAX_TOKENS = 100  # Maximum tokens in the response
TIMEOUT_SECONDS = 10  # Timeout for the OpenAI API request
TTS_MODEL = "gpt-4o-mini-tts"  # The cheapest TTS model
TTS_VOICE = "alloy"  # Pick a voice
# ESP32_URL = "http://192.168.137.124/capture"  # Replace with your ESP32's URL  # Added ESP32 URL


def play_and_save_audio(data_bytes):
    """
    Saves the received audio data as a WAV file and plays it back.

    Args:
        data_bytes (bytes): The audio data in bytes.
    """
    audio_data = np.frombuffer(data_bytes, dtype=DTYPE)
    processed_audio_data = audio_data  # No reshaping needed for mono

    # Save the audio to a WAV file
    try:
        write_wav(OUTPUT_WAV_FILENAME, SAMPLE_RATE, processed_audio_data)
        print(f"💾 Audio saved as '{OUTPUT_WAV_FILENAME}'")
    except OSError as e:
        if e.errno == 28:  # Check for "No space left on device" error
            error_message = "❌ Error: No space left on device while saving audio."
            print(error_message)
            traceback.print_exc()  # Print the traceback to the console
            return jsonify({"status": "error", "message": error_message}), 500
        else:
            error_message = f"❌ Error saving WAV file: {e}"
            print(error_message)
            traceback.print_exc()
            return jsonify({"status": "error", "message": error_message}), 500
    except Exception as e:
        error_message = f"❌ Error saving WAV file: {e}"
        print(error_message)
        traceback.print_exc()
        return jsonify({"status": "error", "message": error_message}), 500

    # Play audio
    print("▶️ Playing audio...")
    try:
        sd.play(processed_audio_data, SAMPLE_RATE)
        sd.wait()
        print("✅ Playback done")
    except Exception as e:
        print(f"❌ Error during audio playback: {e}")
        return jsonify({"status": "error", "message": f"Error during audio playback: {e}"}), 500



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
            model="gpt-4.1-mini",  # Use GPT-4.1-mini for the chat response
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



# def send_capture_signal(): # Removed requests
#     """
#     Sends a signal to the ESP32 to capture an image.  Runs in a separate thread.
#     """
#     try:
#         response = requests.get(ESP32_URL)
#         if response.status_code == 200:
#             print("📸 Signal sent to ESP32 successfully!")
#         else:
#             print(f"❌ Error sending signal to ESP32: {response.status_code}")
#     except requests.exceptions.RequestException as e:
#         print(f"❌ Error sending signal to ESP32: {e}")

def send_capture_signal():
    """
    Sends a signal to the ESP32 to capture an image using urllib.request.
    Runs in a separate thread.
    """
    try:
        with urllib.request.urlopen(ESP32_URL) as response:
            if response.getcode() == 200:
                print("📸 Signal sent to ESP32 successfully!")
            else:
                print(f"❌ Error sending signal to ESP32: {response.getcode()}")
    except Exception as e:
        print(f"❌ Error sending signal to ESP32: {e}")



@app.route("/upload_audio", methods=["POST"])
def upload_audio():
    """
    Handles the audio upload, saves it, transcribes it, and gets an OpenAI response.
    """
    if request.method == "POST":
        audio_data = request.get_data()
        print(f"📥 Received {len(audio_data)} bytes")

        # Check if audio data is empty
        if not audio_data:
            return jsonify({"status": "error", "message": "No audio data received"}), 400  # Return error if no data

        # Save and play audio in a separate thread
        threading.Thread(target=play_and_save_audio, args=(audio_data,), daemon=True).start()

        # Save the audio data to a WAV file
        write_wav(OUTPUT_WAV_FILENAME, SAMPLE_RATE, np.frombuffer(audio_data, dtype=DTYPE))

        # Transcribe the audio
        transcription = transcribe_audio(OUTPUT_WAV_FILENAME)
        if not transcription:
            return jsonify({"status": "error", "message": "Audio transcription failed"}), 500

        print(f"📝 Transcribed text: {transcription}")

        # Check for "help" and send signal to ESP32
        if "help" in transcription.lower():
            print("🚨 \'Help\' detected! Sending signal to ESP32...")
            threading.Thread(target=send_capture_signal).start()  # start thuread

        # Get the OpenAI response
        openai_prompt = f"The following is the transcription of a voice recording: {transcription}.  Please provide a concise and informative answer."
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
            print(f"❌ Error saving answer audioto WAV: {e}")
            return jsonify({
                "status": "error",
                "message": "Speech generation failed: Error saving answer audio"
            }), 500

        if speech_data:
            print("🗣️  Speech generated successfully")
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


if __name__ == "__main__":
    print("🎧 Starting server on http://0.0.0.0:12345/upload")
    print(f"🚀 Received audio will be saved to '{os.path.abspath(OUTPUT_WAV_FILENAME)}'")
    app.run(host="0.0.0.0", port=12345)
