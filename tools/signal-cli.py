#!/usr/bin/env python3
"""
SIGNAL Classified Document Interface (signal-cli)
Interactive CLI minigame for the Aethelian Network encryption protocol.

Usage: python tools/signal-cli.py
Command: si        else:
            # Document is locked
            self.print_colored("🔒 DOCUMENT LOCKED", 'red', 'bold')
            self.print_colored("Enter password to unlock:", 'yellow')
            
            password = input(f"{self.colors['white']}Password: {self.colors['reset']}").strip()
            
            if self.try_password(doc, password):
                self.print_colored("✅ Access granted!", 'green')
                self.show_unlocked_document(doc)
            else:
                self.print_colored("❌ Access denied.", 'red')sified document management and decryption interface for SIGNAL operatives.
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
                
                for line in content.split('\n')[:15]:  # Check first 15 lines
                    if 'SEED-' in line and 'CURVE-' in line:
                        # Extract from format: SEED-1234 CURVE-5678 PASSWORDS-3
                        parts = line.split()
                        for part in parts:
                            if part.startswith('SEED-'):
                                seed = part.split('-')[1]
                            elif part.startswith('CURVE-'):
                                curve = part.split('-')[1]
                            elif part.startswith('PASSWORDS-'):
                                passwords_count = int(part.split('-')[1])
                
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
                print(f"Error loading document {file_path}: {e}")
                continue
        
        # Sort documents by name for consistent ordering
        documents.sort(key=lambda x: x['name'])
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
        
        self.print_colored(f"📄 DOCUMENT: {doc['name']}", 'cyan', 'bold')
        self.print_colored("═" * 50, 'blue')
        
        # Check if document is unlocked
        if doc['name'] in self.session_data['discovered_passwords']:
            # Show unlocked document
            self.show_unlocked_document(doc)
        else:
            # Document is locked
            self.print_colored("� DOCUMENT LOCKED", 'red', 'bold')
            self.print_colored("Enter password to unlock:", 'yellow')
            
            password = input(f"{self.colors['white']}Password: {self.colors['reset']}").strip()
            
            if self.try_password(doc, password):
                self.print_colored("✅ Access granted!", 'green')
                self.show_unlocked_document(doc)
            else:
                self.print_colored("❌ Access denied.", 'red')
        
        print()

    def try_password(self, doc: dict, password: str) -> bool:
        """Try a password against a document. Returns True if correct."""
        # Known passwords for each document
        known_passwords = {
            'AETHELIAN_NETWORK': ['aethelia'],
            'BLACK_ARMADA': ['aethelia', 'shadowhands', 'voidcrown'],
            'NOVA_HEGEMONY': ['stellarfire'],
            'CRIMSON_SYNDICATE': ['bloodmoney', 'crimsoncode'],
            'GHOST_PROTOCOLS': ['whisper'],
            'MONUMENT_ARCHAEOLOGY': ['ancientstone', 'archaeology'],
            'FTL_RESEARCH': ['lightspeed', 'prometheus', 'warpcore']
        }
        
        if doc['name'] in known_passwords:
            if password in known_passwords[doc['name']]:
                # Store all passwords for this document (simplified approach)
                self.session_data['discovered_passwords'][doc['name']] = known_passwords[doc['name']]
                return True
        
        return False

    def show_unlocked_document(self, doc: dict):
        """Show the full unlocked document content."""
        try:
            passwords = self.session_data['discovered_passwords'][doc['name']]
            decrypted_content = decrypt_with_passwords(
                str(doc['file_path']),
                doc['seed'],
                doc['curve'],
                doc['passwords_count'],
                passwords,
                str(self.wordlist_path)
            )
            
            self.print_colored("📖 DOCUMENT CONTENT:", 'green', 'bold')
            self.print_colored("─" * 50, 'gray')
            
            # Show the decrypted content
            lines = decrypted_content.split('\n')[12:]  # Skip headers
            for line in lines[:30]:  # Show first 30 lines
                if line.strip():
                    self.print_colored(line, 'white')
            
            if len(lines) > 30:
                self.print_colored("... (content continues) ...", 'gray')
                
        except Exception as e:
            self.print_colored("❌ Error displaying document", 'red')

    # Function removed - decryption now handled in show_document_detail

    def show_full_document(self, doc: Dict, content: str = ""):
        """Display the fully or partially decrypted document."""
        if not content:
            try:
                passwords = self.session_data['discovered_passwords'].get(doc['name'], [])
                content = decrypt_with_passwords(
                    str(doc['file_path']),
                    doc['seed'],
                    doc['curve'],
                    doc['passwords_count'],
                    passwords,
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
        """Encryption not available in simplified mode."""
        self.print_colored("❌ Encryption feature not available", 'red')

    def show_help(self):
        """Display help and command reference."""
        self.print_colored("📚 SIGNAL INTERFACE HELP", 'cyan', 'bold')
        self.print_colored("═" * 80, 'blue')
        
        commands = [
            ("list (l)", "Show all classified documents"),
            ("view <index> (v)", "View document (will ask for password if locked)"),
            ("session", "Show session information"),
            ("clear", "Clear the terminal"),
            ("help (h)", "Show this help"),
            ("exit (q)", "Exit the interface")
        ]
        
        for cmd, desc in commands:
            self.print_colored(f"  {cmd:<20} - {desc}", 'white')
        
        print()
        self.print_colored("� PASSWORD HINTS:", 'cyan', 'bold')
        self.print_colored("  • 'aethelia' relates to the ancient network builders", 'gray')
        self.print_colored("  • 'shadowhands' - those who work in darkness", 'gray')  
        self.print_colored("  • 'voidcrown' - the unseen rulers of the void", 'gray')
        
        print()
        self.print_colored("�💡 TIPS:", 'yellow', 'bold')
        self.print_colored("  • Passwords are discovered through gameplay and lore exploration", 'gray')
        self.print_colored("  • Each document has multiple encryption layers", 'gray')
        self.print_colored("  • Progressive decryption reveals more content with each password", 'gray')
        self.print_colored("  • Type 'demo' to try all known passwords (spoilers!)", 'gray')
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
