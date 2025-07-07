#!/usr/bin/env python3
"""
Simple HTTP server for serving WASM build with correct MIME types
"""

import http.server
import os
import sys

class WASMHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        super().end_headers()
    
    def guess_type(self, path):
        mimetype = super().guess_type(path)
        if path.endswith('.wasm'):
            return 'application/wasm'
        return mimetype

if __name__ == '__main__':
    os.chdir('build')
    PORT = 8000
    
    print(f"🌐 Serving WASM build at http://localhost:{PORT}/signal.html")
    print("📋 Press Ctrl+C to stop the server")
    
    with http.server.HTTPServer(("", PORT), WASMHTTPRequestHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n🛑 Server stopped")
            sys.exit(0)