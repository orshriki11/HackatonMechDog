from http.server import BaseHTTPRequestHandler, HTTPServer

HOST = '0.0.0.0'
PORT = 12345

class ESP32Handler(BaseHTTPRequestHandler):
    def do_POST(self):
        if self.path == '/upload':
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length)

            # Try to decode as UTF-8, fallback to raw bytes
            try:
                message = post_data.decode('utf-8')
                print(f"📥 Received (text): {message}")
            except UnicodeDecodeError:
                print(f"📥 Received (binary): {post_data.hex()}")

            # Respond with text
            response = "✅ Server received your message"
            self.send_response(200)
            self.send_header('Content-Type', 'text/plain')
            self.send_header('Content-Length', str(len(response)))
            self.end_headers()
            self.wfile.write(response.encode())
        else:
            self.send_error(404, "Not Found")

def run():
    print(f"🟢 HTTP server running at http://{HOST}:{PORT}/upload")
    server = HTTPServer((HOST, PORT), ESP32Handler)
    server.serve_forever()

if __name__ == "__main__":
    run()
