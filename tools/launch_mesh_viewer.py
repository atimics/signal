#!/usr/bin/env python3
"""
CGame Mesh Viewer Launcher

Starts a local web server and opens the mesh viewer in the default browser.
"""

import subprocess
import webbrowser
import time
import sys
import signal
import os
from pathlib import Path

def main():
    # Ensure we're in the correct directory
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    
    if not (project_root / "assets" / "meshes").exists():
        print("❌ Error: Must run from the cgame project root directory")
        print(f"   Current directory: {os.getcwd()}")
        print(f"   Expected assets at: {project_root / 'assets' / 'meshes'}")
        return 1
    
    # Change to project root
    os.chdir(project_root)
    
    print("🚀 CGame Mesh Viewer Launcher")
    print("=" * 40)
    print(f"📁 Project root: {project_root}")
    print(f"🌐 Starting web server...")
    
    # Start the web server
    port = 8080
    server_url = f"http://127.0.0.1:{port}"
    viewer_url = f"{server_url}/tools/mesh_viewer/"
    
    try:
        # Start server in background
        server_process = subprocess.Popen([
            sys.executable, "-m", "http.server", str(port), "--bind", "127.0.0.1"
        ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
        # Wait a moment for server to start
        time.sleep(2)
        
        print(f"✅ Server running at: {server_url}")
        print(f"🎨 Opening mesh viewer: {viewer_url}")
        print()
        print("📋 Controls:")
        print("   • Click meshes in sidebar to view them")
        print("   • Use mouse to orbit, zoom, and pan")
        print("   • Texture preview shows in bottom-right")
        print("   • Press Ctrl+C to stop the server")
        print()
        
        # Open browser
        webbrowser.open(viewer_url)
        
        # Keep server running until interrupted
        try:
            server_process.wait()
        except KeyboardInterrupt:
            print("\n🛑 Stopping server...")
            server_process.terminate()
            server_process.wait()
            print("✅ Server stopped")
            return 0
            
    except Exception as e:
        print(f"❌ Error starting server: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())
