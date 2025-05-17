import wave

def save_as_wav(raw_bytes, filename, sample_rate=16000, sample_width=2, channels=1):
    with wave.open(filename, 'wb') as wf:
        wf.setnchannels(channels)
        wf.setsampwidth(sample_width)
        wf.setframerate(sample_rate)
        wf.writeframes(raw_bytes)