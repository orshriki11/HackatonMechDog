from http.server import BaseHTTPRequestHandler, HTTPServer

class SimpleWavServer(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        wav_data = self.rfile.read(content_length)

        with open("received_audio.wav", "wb") as f:
            f.write(wav_data)

        self.send_response(200)
        self.end_headers()
        self.wfile.write(b"WAV file received and saved.")

if __name__ == "__main__":
    server_address = ("0.0.0.0", 12345)
    httpd = HTTPServer(server_address, SimpleWavServer)
    print("Listening on port 12345...")
    httpd.serve_forever()