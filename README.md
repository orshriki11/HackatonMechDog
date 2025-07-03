# 🐕 CHASE: AIoT Helper Robot for Distress Situations


> 🚨 An intelligent, voice-activated robotic dog built to assist people in distress, developed in a 30-hour AIoT hackathon at the Technion.

![pictures](https://github.com/user-attachments/assets/bc99c1c3-cbfc-4477-a081-9bfe952227b9)

## 🧠 Motivation

In emergencies such as falls among the elderly or post-earthquake search-and-rescue missions, time is critical. This project aims to demonstrate a **proof-of-concept** for a compact, AI-driven robotic assistant that can **hear**, **understand**, and **respond** to distress situations in real time — bridging the gap between autonomy and empathy.

---

## 🎯 Project Overview

CHASE is an **AIoT-powered quadruped robot** designed to:
- Detect verbal help calls using **speech-to-text (STT)**
- Identify people visually using **image recognition**
- Navigate autonomously to the person in distress
- Deliver a natural-language response using **OpenAI GPT**

All of this is orchestrated using a **distributed microcontroller system** and a central Python server.

---

## ⚙️ System Architecture

The system is composed of:

### 1. **ESP32 Microcontrollers**
Each module runs C++ firmware and acts as a Wi-Fi client communicating with the Python server:
- 🎙️ **Microphone Unit:** Records audio upon button press and sends it to the server
- 📷 **Camera Unit:** Captures and sends images for visual recognition
- 🐾 **MechDog Unit:** Controls the Hiwonder MechDog robot movement and ultrasonic-based proximity handling

### 2. **Python Control Server**
Central coordination node that:
- Orchestrates MCU interactions
- Sends and receives commands via Wi-Fi
- Handles API requests to **OpenAI** (STT + GPT + Vision)

### 3. **AI Capabilities**
- 🗣️ **Speech Recognition:** Identifies help calls using OpenAI Whisper or similar STT
- 👁️ **Image Classification:** Verifies presence of a person using OpenAI Vision APIs
- 💬 **GPT-Based Response:** Generates context-aware verbal responses

---

## 🚀 Demo Workflow

1. User presses a button and says: *“Help, I’ve fallen!”*
2. Audio is recorded and sent to the Python server
3. Server uses OpenAI STT to determine intent
4. If a distress call is detected:
    - Server commands the MechDog to approach the person
    - Camera takes an image and checks for a person
    - If identified, MechDog moves and stops at a safe distance (using an ultrasonic sensor)
    - GPT generates a comforting or informative response, played back to the user

---

## 🛠️ Technologies Used

- **Hardware:**  
  - Hiwonder MechDog  
  - ESP32 microcontrollers (3 units)  
  - Ultrasonic sensor  
  - Microphone
  - Camera module

- **Firmware:**  
  - C++ (PlatformIO/Arduino)

- **Server:**  
  - Python 3.x  
  - Async socket communication  
  - OpenAI API (STT, GPT, Vision)

- **Communication:**  
  - Wi-Fi  
  - TCP sockets
  - I2S communication with microphone sensor

---

## 🧪 Getting Started
This project is a POC created during a hackathon. For future deployment, proper modularization and robustness are recommended.

1. Clone the repo

bash
```
git clone https://github.com/orshriki11/HackatonMechDog.git
cd HackatonMechDog
```
2. Flash Firmware to Each ESP32
- Use PlatformIO or Arduino IDE
- Update your Wi-Fi credentials in each firmware's config

3. Run Python Server
   ```
   cd server files
   python mechdog_main_server.py
   ```
5. Configure API Keys
Make sure to add your OpenAI API key to the environment or config file

---
## 🙌 Credits
Developed with passion at the Technion AIoT Hackathon (2025) in just 30 hours.

### Creators

Or Shriki and Roee Marom.

Special thanks to the mentors and organizers who made this experience unforgettable — your guidance and energy helped bring CHASE to life.





