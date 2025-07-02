#!/usr/bin/env python3
"""
SIGNAL Classified Document Interface (signal-cli)
Interactive CLI minigame for the Aethelian Network encryption protocol.

Usage: python tools/signal-cli.py
Command: signal

A classified document management and decryption interface for SIGNAL operatives.
"""

import os
import sys
import json
import time
import random
from typing import List, Dict, Optional, Tuple
from pathlib import Path

# Add tools directory to path for imports
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

try:
    from decrypt_document import DocumentDecryptor, load_word_list, decrypt_with_passwords
    from encrypt_document import DocumentEncryptor, encrypt_document_with_params
except ImportError:
    print("ERROR: Missing encryption modules. Ensure decrypt_document.py and encrypt_document.py are available.")
    sys.exit(1)


class SIGNALInterface:
    """SIGNAL Classified Document Interface - Interactive CLI for document encryption/decryption."""
    
    def __init__(self):
        self.base_dir = Path(__file__).parent.parent
        self.classified_dir = self.base_dir / "docs" / "SIGNAL" / "classified"
        self.wordlist_path = self.base_dir / "tools" / "signal_wordlist.txt"
        self.session_data = {
            "discovered_passwords": {},
            "access_level": "STANDARD",
            "session_start": time.time()
        }
        
        # ASCII Art and styling
        self.logo = """
╔═══════════════════════════════════════════════════════════════════════════════╗
║  ███████╗██╗ ██████╗ ███╗   ██╗ █████╗ ██╗         ██████╗██╗     ██╗       ║
║  ██╔════╝██║██╔════╝ ████╗  ██║██╔══██╗██║        ██╔════╝██║     ██║       ║
║  ███████╗██║██║  ███╗██╔██╗ ██║███████║██║        ██║     ██║     ██║       ║
║  ╚════██║██║██║   ██║██║╚██╗██║██╔══██║██║        ██║     ██║     ██║       ║
║  ███████║██║╚██████╔╝██║ ╚████║██║  ██║███████╗   ╚██████╗███████╗██║       ║
║  ╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═╝╚══════╝    ╚═════╝╚══════╝╚═╝       ║
║                                                                               ║
║                    CLASSIFIED DOCUMENT INTERFACE                             ║
║                         Aethelian Network Protocol                           ║
╚═══════════════════════════════════════════════════════════════════════════════╝
        """
        
        self.colors = {
            'red': '\033[91m',
            'green': '\033[92m',
            'yellow': '\033[93m',
            'blue': '\033[94m',
            'magenta': '\033[95m',
            'cyan': '\033[96m',
            'white': '\033[97m',
            'gray': '\033[90m',
            'bold': '\033[1m',
            'dim': '\033[2m',
            'reset': '\033[0m'
        }

    def print_colored(self, text: str, color: str = 'white', style: str = ''):
        """Print colored text with optional styling."""
        color_code = self.colors.get(color, self.colors['white'])
        style_code = self.colors.get(style, '')
        print(f"{style_code}{color_code}{text}{self.colors['reset']}")

    def print_typewriter(self, text: str, delay: float = 0.03, color: str = 'white'):
        """Print text with typewriter effect."""
        color_code = self.colors.get(color, self.colors['white'])
        for char in text:
            print(f"{color_code}{char}{self.colors['reset']}", end='', flush=True)
            time.sleep(delay)
        print()

    def clear_screen(self):
        """Clear the terminal screen."""
        os.system('cls' if os.name == 'nt' else 'clear')

    def show_header(self):
        """Display the SIGNAL interface header."""
        self.clear_screen()
        self.print_colored(self.logo, 'cyan')
        self.print_colored(f"SESSION ID: {int(self.session_data['session_start'])}", 'gray')
        self.print_colored(f"ACCESS LEVEL: {self.session_data['access_level']}", 'yellow')
        self.print_colored(f"OPERATIVE STATUS: ACTIVE", 'green')
        print()

    def load_classified_documents(self) -> List[Dict]:
        """Load and parse available classified documents."""
        documents = []
        
        if not self.classified_dir.exists():
            return documents
            
        # Load encryption metadata
        metadata_file = self.classified_dir / "encryption_metadata.json"
        metadata = {}
        if metadata_file.exists():
            try:
                with open(metadata_file, 'r') as f:
                    metadata = json.load(f)
            except:
                pass
        
        # Scan for encrypted documents
        for file_path in self.classified_dir.glob("*_ENCRYPTED.md"):
            doc_name = file_path.stem.replace("_ENCRYPTED", "")
            
            # Parse document header for encryption info
            try:
                with open(file_path, 'r') as f:
                    content = f.read()
                    
                # Extract encryption parameters from header
                seed = None
                curve = None
                passwords_count = None
                
                for line in content.split('\n')[:10]:
                    if "SEED-" in line and "CURVE-" in line:
                        parts = line.split()
                        for part in parts:
                            if part.startswith("SEED-"):
                                seed = part.split("-")[1]
                            elif part.startswith("CURVE-"):
                                curve = part.split("-")[1]
                            elif part.startswith("PASSWORDS-"):
                                passwords_count = int(part.split("-")[1])
                
                if seed and curve and passwords_count is not None:
                    documents.append({
                        'name': doc_name,
                        'file_path': file_path,
                        'seed': seed,
                        'curve': curve,
                        'passwords_count': passwords_count,
                        'discovered_passwords': self.session_data['discovered_passwords'].get(doc_name, [])
                    })
                    
            except Exception as e:
                continue
                
        return documents

    def show_document_list(self):
        """Display available classified documents."""
        documents = self.load_classified_documents()
        
        if not documents:
            self.print_colored("⚠️  NO CLASSIFIED DOCUMENTS FOUND", 'red', 'bold')
            self.print_colored("Check that encrypted documents exist in docs/SIGNAL/classified/", 'gray')
            return
        
        self.print_colored("📂 CLASSIFIED DOCUMENT ARCHIVE", 'cyan', 'bold')
        self.print_colored("═" * 80, 'blue')
        
        for i, doc in enumerate(documents, 1):
            status_icon = "🔒" if len(doc['discovered_passwords']) == 0 else "🔓"
            progress = len(doc['discovered_passwords'])
            total = doc['passwords_count']
            
            # Calculate readability percentage
            readability = (progress / total * 100) if total > 0 else 0
            
            color = 'red' if readability == 0 else 'yellow' if readability < 100 else 'green'
            
            self.print_colored(f"{i:2d}. {status_icon} {doc['name']}", color, 'bold')
            self.print_colored(f"     Classification: VOID BLACK", 'gray')
            self.print_colored(f"     Encryption: SEED-{doc['seed']} CURVE-{doc['curve']}", 'gray')
            self.print_colored(f"     Progress: {progress}/{total} keys ({readability:.1f}% readable)", color)
            print()

    def show_document_detail(self, doc_index: int):
        """Show detailed view of a specific document."""
        documents = self.load_classified_documents()
        
        if doc_index < 1 or doc_index > len(documents):
            self.print_colored("❌ Invalid document index", 'red')
            return
            
        doc = documents[doc_index - 1]
        progress = len(doc['discovered_passwords'])
        total = doc['passwords_count']
        readability = (progress / total * 100) if total > 0 else 0
        
        self.print_colored(f"📄 DOCUMENT ANALYSIS: {doc['name']}", 'cyan', 'bold')
        self.print_colored("═" * 80, 'blue')
        
        # Show encryption details
        self.print_colored(f"🔐 ENCRYPTION PARAMETERS:", 'yellow')
        self.print_colored(f"   Seed: {doc['seed']}", 'white')
        self.print_colored(f"   Curve: {doc['curve']}", 'white')
        self.print_colored(f"   Password Layers: {doc['passwords_count']}", 'white')
        print()
        
        # Show progress
        self.print_colored(f"📊 DECRYPTION PROGRESS:", 'yellow')
        self.print_colored(f"   Discovered Keys: {progress}/{total}", 'white')
        self.print_colored(f"   Estimated Readability: {readability:.1f}%", 'white')
        
        if doc['discovered_passwords']:
            self.print_colored(f"   Active Passwords: {', '.join(['*' * len(p) for p in doc['discovered_passwords']])}", 'green')
        print()
        
        # Show preview with current decryption level
        self.print_colored(f"📖 DOCUMENT PREVIEW (Current Decryption Level):", 'yellow')
        self.print_colored("─" * 80, 'gray')
        
        try:
            # Attempt to decrypt with current passwords
            if doc['discovered_passwords']:
                decrypted_content = decrypt_with_passwords(
                    str(doc['file_path']),
                    doc['seed'],
                    doc['curve'],
                    doc['passwords_count'],
                    doc['discovered_passwords'],
                    str(self.wordlist_path)
                )
                
                # Show first few lines
                lines = decrypted_content.split('\n')
                preview_lines = lines[15:25]  # Skip metadata, show content
                
                for line in preview_lines:
                    if line.strip():
                        # Highlight encrypted tokens
                        if '[' in line and ']' in line:
                            self.print_colored(line, 'red', 'dim')
                        else:
                            self.print_colored(line, 'white')
            else:
                # Show encrypted version
                with open(doc['file_path'], 'r') as f:
                    lines = f.readlines()[15:25]
                    for line in lines:
                        if line.strip():
                            self.print_colored(line.strip(), 'red', 'dim')
                            
        except Exception as e:
            self.print_colored("❌ Error reading document preview", 'red')
        
        print()

    def attempt_decryption(self, doc_index: int):
        """Interactive decryption attempt interface."""
        documents = self.load_classified_documents()
        
        if doc_index < 1 or doc_index > len(documents):
            self.print_colored("❌ Invalid document index", 'red')
            return
            
        doc = documents[doc_index - 1]
        current_passwords = doc['discovered_passwords'].copy()
        
        self.print_colored(f"🔓 ATTEMPTING DECRYPTION: {doc['name']}", 'cyan', 'bold')
        self.print_colored("═" * 80, 'blue')
        
        # Show current status
        progress = len(current_passwords)
        total = doc['passwords_count']
        
        if progress >= total:
            self.print_colored("✅ Document already fully decrypted!", 'green')
            self.show_full_document(doc)
            return
        
        self.print_colored(f"Current progress: {progress}/{total} password layers", 'yellow')
        self.print_colored(f"Enter password for layer {progress + 1}:", 'white')
        
        # Create dramatic input prompt
        self.print_typewriter("Accessing Aethelian Network...", 0.05, 'cyan')
        self.print_typewriter("Establishing secure connection...", 0.05, 'cyan')
        self.print_typewriter("Ready for password input.", 0.05, 'green')
        
        print()
        password = input(f"{self.colors['yellow']}ENTER PASSWORD > {self.colors['reset']}").strip()
        
        if not password:
            self.print_colored("❌ Password cannot be empty", 'red')
            return
        
        # Test the password
        test_passwords = current_passwords + [password]
        
        self.print_typewriter("Validating password...", 0.08, 'yellow')
        self.print_typewriter("Checking against Aethelian cipher...", 0.08, 'yellow')
        
        try:
            # Attempt decryption with new password
            decrypted_content = decrypt_with_passwords(
                str(doc['file_path']),
                doc['seed'],
                doc['curve'],
                doc['passwords_count'],
                test_passwords,
                str(self.wordlist_path)
            )
            
            # Count decrypted tokens to verify progress
            original_content = open(doc['file_path'], 'r').read()
            original_tokens = original_content.count('[')
            current_tokens = decrypted_content.count('[')
            
            if current_tokens < original_tokens:
                # Password worked!
                self.print_typewriter("PASSWORD ACCEPTED!", 0.1, 'green')
                self.print_colored("✅ Decryption layer unlocked!", 'green', 'bold')
                
                # Update session data
                if doc['name'] not in self.session_data['discovered_passwords']:
                    self.session_data['discovered_passwords'][doc['name']] = []
                self.session_data['discovered_passwords'][doc['name']] = test_passwords
                
                new_progress = len(test_passwords)
                new_readability = (new_progress / total * 100)
                
                self.print_colored(f"🔓 New progress: {new_progress}/{total} ({new_readability:.1f}% readable)", 'green')
                
                if new_progress >= total:
                    self.print_colored("🎉 DOCUMENT FULLY DECRYPTED!", 'green', 'bold')
                    self.show_full_document(doc, decrypted_content)
                else:
                    self.print_colored(f"📈 Layer {new_progress} unlocked. {total - new_progress} layers remaining.", 'yellow')
                    
            else:
                self.print_typewriter("INVALID PASSWORD", 0.1, 'red')
                self.print_colored("❌ Password rejected by Aethelian cipher", 'red')
                
        except Exception as e:
            self.print_colored(f"❌ Decryption error: {str(e)}", 'red')

    def show_full_document(self, doc: Dict, content: str = None):
        """Display the fully or partially decrypted document."""
        if content is None:
            try:
                content = decrypt_with_passwords(
                    str(doc['file_path']),
                    doc['seed'],
                    doc['curve'],
                    doc['passwords_count'],
                    doc['discovered_passwords'],
                    str(self.wordlist_path)
                )
            except Exception as e:
                self.print_colored(f"❌ Error loading document: {str(e)}", 'red')
                return
        
        self.print_colored(f"📄 CLASSIFIED DOCUMENT: {doc['name']}", 'cyan', 'bold')
        self.print_colored("═" * 80, 'blue')
        
        # Skip metadata lines and show content
        lines = content.split('\n')[15:]  # Skip encryption metadata
        
        for line in lines:
            if line.strip():
                # Color-code based on encryption status
                if '[' in line and ']' in line:
                    # Still encrypted
                    self.print_colored(line, 'red', 'dim')
                elif line.startswith('#'):
                    # Headers
                    self.print_colored(line, 'cyan', 'bold')
                elif line.startswith('**'):
                    # Bold sections
                    self.print_colored(line, 'yellow', 'bold')
                else:
                    # Regular text
                    self.print_colored(line, 'white')
        
        print()

    def encrypt_new_document(self):
        """Interface for encrypting new documents."""
        self.print_colored("🔐 DOCUMENT ENCRYPTION INTERFACE", 'cyan', 'bold')
        self.print_colored("═" * 80, 'blue')
        
        # Get input file
        print()
        self.print_colored("Enter path to document to encrypt:", 'yellow')
        input_file = input(f"{self.colors['white']}FILE PATH > {self.colors['reset']}").strip()
        
        if not input_file or not os.path.exists(input_file):
            self.print_colored("❌ File not found", 'red')
            return
        
        # Get encryption parameters
        print()
        self.print_colored("Configure encryption parameters:", 'yellow')
        
        try:
            seed = input(f"{self.colors['white']}Seed (4 digits) > {self.colors['reset']}").strip()
            if len(seed) != 4 or not seed.isdigit():
                raise ValueError("Seed must be 4 digits")
            
            curve = input(f"{self.colors['white']}Curve (4 digits) > {self.colors['reset']}").strip()
            if len(curve) != 4 or not curve.isdigit():
                raise ValueError("Curve must be 4 digits")
            
            password_count = int(input(f"{self.colors['white']}Number of password layers > {self.colors['reset']}").strip())
            if password_count < 1 or password_count > 10:
                raise ValueError("Password count must be 1-10")
            
            passwords = []
            for i in range(password_count):
                pwd = input(f"{self.colors['white']}Password {i+1} > {self.colors['reset']}").strip()
                if not pwd:
                    raise ValueError(f"Password {i+1} cannot be empty")
                passwords.append(pwd)
                
        except (ValueError, KeyboardInterrupt) as e:
            self.print_colored(f"❌ Invalid input: {str(e)}", 'red')
            return
        
        # Generate output filename
        input_path = Path(input_file)
        output_file = self.classified_dir / f"{input_path.stem}_ENCRYPTED.md"
        
        self.print_typewriter("Initializing Aethelian encryption protocol...", 0.05, 'cyan')
        self.print_typewriter("Generating encryption matrix...", 0.05, 'cyan')
        self.print_typewriter("Applying progressive cipher layers...", 0.05, 'cyan')
        
        try:
            # Perform encryption
            encrypt_document_with_params(
                input_file,
                str(output_file),
                seed,
                curve,
                password_count,
                passwords,
                str(self.wordlist_path)
            )
            
            self.print_colored("✅ Document encrypted successfully!", 'green', 'bold')
            self.print_colored(f"📁 Output: {output_file}", 'green')
            self.print_colored(f"🔐 Parameters: SEED-{seed} CURVE-{curve} PASSWORDS-{password_count}", 'yellow')
            
        except Exception as e:
            self.print_colored(f"❌ Encryption failed: {str(e)}", 'red')

    def show_help(self):
        """Display help and command reference."""
        self.print_colored("📚 SIGNAL INTERFACE HELP", 'cyan', 'bold')
        self.print_colored("═" * 80, 'blue')
        
        commands = [
            ("list", "Show all classified documents"),
            ("view <index>", "View detailed document information"),
            ("decrypt <index>", "Attempt to decrypt a document"),
            ("encrypt", "Encrypt a new document"),
            ("session", "Show session information"),
            ("clear", "Clear the terminal"),
            ("help", "Show this help"),
            ("exit", "Exit the interface")
        ]
        
        for cmd, desc in commands:
            self.print_colored(f"  {cmd:<15} - {desc}", 'white')
        
        print()
        self.print_colored("💡 TIPS:", 'yellow', 'bold')
        self.print_colored("  • Passwords are discovered through gameplay and lore exploration", 'gray')
        self.print_colored("  • Each document has multiple encryption layers", 'gray')
        self.print_colored("  • Progressive decryption reveals more content with each password", 'gray')
        self.print_colored("  • Check the game repository for password hints", 'gray')
        print()

    def show_session_info(self):
        """Display current session information."""
        session_time = time.time() - self.session_data['session_start']
        
        self.print_colored("📊 SESSION INFORMATION", 'cyan', 'bold')
        self.print_colored("═" * 80, 'blue')
        
        self.print_colored(f"Session Duration: {int(session_time // 60)}m {int(session_time % 60)}s", 'white')
        self.print_colored(f"Access Level: {self.session_data['access_level']}", 'yellow')
        self.print_colored(f"Documents Accessed: {len(self.session_data['discovered_passwords'])}", 'white')
        
        if self.session_data['discovered_passwords']:
            print()
            self.print_colored("🔓 DISCOVERED PASSWORDS:", 'green', 'bold')
            for doc_name, passwords in self.session_data['discovered_passwords'].items():
                self.print_colored(f"  {doc_name}: {len(passwords)} keys", 'green')
        
        print()

    def run(self):
        """Main interface loop."""
        self.show_header()
        
        self.print_typewriter("Initializing SIGNAL classified document interface...", 0.05, 'cyan')
        self.print_typewriter("Connecting to Aethelian Network...", 0.05, 'cyan')
        self.print_typewriter("Authentication verified. Welcome, operative.", 0.05, 'green')
        
        print()
        self.print_colored("Type 'help' for available commands.", 'gray')
        print()
        
        while True:
            try:
                command = input(f"{self.colors['cyan']}SIGNAL > {self.colors['reset']}").strip().lower()
                
                if not command:
                    continue
                    
                parts = command.split()
                cmd = parts[0]
                
                if cmd in ['exit', 'quit', 'q']:
                    self.print_typewriter("Terminating session...", 0.05, 'yellow')
                    self.print_typewriter("Connection closed. Stay vigilant, operative.", 0.05, 'cyan')
                    break
                    
                elif cmd in ['help', 'h']:
                    self.show_help()
                    
                elif cmd in ['list', 'ls']:
                    self.show_document_list()
                    
                elif cmd in ['view', 'v']:
                    if len(parts) > 1:
                        try:
                            index = int(parts[1])
                            self.show_document_detail(index)
                        except ValueError:
                            self.print_colored("❌ Invalid document index", 'red')
                    else:
                        self.print_colored("Usage: view <index>", 'yellow')
                        
                elif cmd in ['decrypt', 'd']:
                    if len(parts) > 1:
                        try:
                            index = int(parts[1])
                            self.attempt_decryption(index)
                        except ValueError:
                            self.print_colored("❌ Invalid document index", 'red')
                    else:
                        self.print_colored("Usage: decrypt <index>", 'yellow')
                        
                elif cmd in ['encrypt', 'e']:
                    self.encrypt_new_document()
                    
                elif cmd in ['session', 'info']:
                    self.show_session_info()
                    
                elif cmd in ['clear', 'cls']:
                    self.show_header()
                    
                else:
                    self.print_colored(f"❌ Unknown command: {cmd}", 'red')
                    self.print_colored("Type 'help' for available commands.", 'gray')
                
                print()
                
            except KeyboardInterrupt:
                print()
                self.print_colored("Session interrupted. Type 'exit' to quit.", 'yellow')
                print()
            except EOFError:
                print()
                break


def main():
    """Entry point for signal-cli."""
    if len(sys.argv) > 1 and sys.argv[1] in ['-h', '--help']:
        print("SIGNAL Classified Document Interface")
        print("Usage: python tools/signal-cli.py")
        print("       python tools/signal-cli.py")
        print()
        print("Interactive CLI for managing and decrypting SIGNAL classified documents.")
        return
    
    try:
        interface = SIGNALInterface()
        interface.run()
    except Exception as e:
        print(f"FATAL ERROR: {str(e)}")
        sys.exit(1)


if __name__ == "__main__":
    main()
