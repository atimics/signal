#!/usr/bin/env python3
"""
SIGNAL: Aethelian Decryption Terminal
Interactive CLI minigame for document encryption/decryption
"""

import os
import sys
import time
import random
import json
from typing import List, Dict, Optional

# Add current directory to path for imports
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, current_dir)

try:
    from encrypt_document import DocumentEncryptor
    from decrypt_document import DocumentDecryptor, load_word_list
except ImportError:
    print("Error: Could not import encryption modules.")
    print("Make sure encrypt_document.py and decrypt_document.py are in the same directory.")
    sys.exit(1)

class AethelianTerminal:
    def __init__(self):
        self.discovered_passwords = []
        self.available_documents = self._scan_classified_documents()
        self.word_list = self._load_wordlist()
        self.hacking_progress = 0
        
    def _scan_classified_documents(self) -> Dict[str, Dict]:
        """Scan for available classified documents"""
        docs = {}
        classified_dir = "docs/SIGNAL/classified"
        
        if os.path.exists(classified_dir):
            for filename in os.listdir(classified_dir):
                if filename.endswith("_ENCRYPTED.md"):
                    filepath = os.path.join(classified_dir, filename)
                    try:
                        with open(filepath, 'r') as f:
                            content = f.read()
                        
                        # Initialize defaults
                        seed = "0000"
                        curve = "0000" 
                        passwords = 0
                        
                        # Extract encryption parameters from header
                        if "ENCRYPTION STAMP" in content:
                            for line in content.split('\n'):
                                if "ENCRYPTION STAMP" in line:
                                    parts = line.split()
                                    for part in parts:
                                        if part.startswith("SEED-"):
                                            seed = part.split("-")[1]
                                        elif part.startswith("CURVE-"):
                                            curve = part.split("-")[1]
                                        elif part.startswith("PASSWORDS-"):
                                            passwords = int(part.split("-")[1])
                        
                        docs[filename] = {
                            'filepath': filepath,
                            'seed': seed,
                            'curve': curve,
                            'passwords': passwords,
                            'content': content
                        }
                    except Exception as e:
                        continue
        
        return docs
    
    def _load_wordlist(self) -> List[str]:
        """Load decryption wordlist"""
        wordlist_path = "tools/signal_wordlist.txt"
        if os.path.exists(wordlist_path):
            return load_word_list(wordlist_path)
        else:
            return load_word_list()  # Use default wordlist
    
    def _typewriter_print(self, text: str, delay: float = 0.03):
        """Print text with typewriter effect"""
        for char in text:
            print(char, end='', flush=True)
            time.sleep(delay)
        print()
    
    def _loading_animation(self, text: str, duration: float = 2.0):
        """Show loading animation"""
        chars = "⠁⠂⠄⡀⢀⠠⠐⠈"
        start_time = time.time()
        i = 0
        
        while time.time() - start_time < duration:
            print(f"\r{chars[i % len(chars)]} {text}", end='', flush=True)
            time.sleep(0.1)
            i += 1
        
        print(f"\r✓ {text}")
    
    def _hack_simulation(self, difficulty: int = 3):
        """Simulate hacking minigame"""
        print("\n" + "="*60)
        self._typewriter_print("INITIATING NEURAL INTERFACE...", 0.05)
        self._loading_animation("Bypassing firewalls", 1.5)
        self._loading_animation("Cracking encryption layers", 2.0)
        
        # Mini puzzle game
        print(f"\n🔐 DECRYPTION CHALLENGE - DIFFICULTY LEVEL {difficulty}")
        print("Complete the sequence to proceed:")
        
        # Generate a simple pattern matching game
        sequence = [random.randint(1, 4) for _ in range(difficulty)]
        symbols = ["▲", "●", "■", "♦"]
        
        print("Pattern:", " ".join(symbols[i-1] for i in sequence))
        
        attempts = 3
        while attempts > 0:
            user_input = input(f"Enter sequence (1-4, space separated): ").strip()
            try:
                user_sequence = [int(x) for x in user_input.split()]
                if user_sequence == sequence:
                    self._typewriter_print("✓ DECRYPTION SUCCESSFUL", 0.05)
                    self.hacking_progress += 1
                    return True
                else:
                    attempts -= 1
                    if attempts > 0:
                        print(f"❌ Incorrect. {attempts} attempts remaining.")
                    else:
                        print("❌ DECRYPTION FAILED - ACCESS DENIED")
                        return False
            except ValueError:
                attempts -= 1
                print(f"❌ Invalid input. {attempts} attempts remaining.")
        
        return False
    
    def show_splash_screen(self):
        """Display terminal splash screen"""
        os.system('clear' if os.name == 'posix' else 'cls')
        
        splash = """
╔══════════════════════════════════════════════════════════════╗
║                                                              ║
║    ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄  ▄▄▄▄▄▄▄▄▄▄▄  ▄▄        ▄▄      ║
║   ▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░▌      ▐░░▌     ║
║   ▐░█▀▀▀▀▀▀▀▀▀ ▐░█▀▀▀▀▀▀▀█░▌▐░█▀▀▀▀▀▀▀▀▀ ▐░▌░▌     ▐░▌░▌     ║
║   ▐░▌          ▐░▌       ▐░▌▐░▌          ▐░▌▐░▌    ▐░▌▐░▌    ║
║   ▐░█▄▄▄▄▄▄▄▄▄ ▐░▌       ▐░▌▐░▌ ▄▄▄▄▄▄▄▄ ▐░▌ ▐░▌   ▐░▌ ▐░▌   ║
║   ▐░░░░░░░░░░░▌▐░▌       ▐░▌▐░▌▐░░░░░░░░▌▐░▌  ▐░▌  ▐░▌  ▐░▌  ║
║    ▀▀▀▀▀▀▀▀▀█░▌▐░▌       ▐░▌▐░▌ ▀▀▀▀▀▀█░▌▐░▌   ▐░▌ ▐░▌   ▐░▌ ║
║             ▐░▌▐░▌       ▐░▌▐░▌       ▐░▌▐░▌    ▐░▌▐░▌    ▐░▌║
║    ▄▄▄▄▄▄▄▄▄█░▌▐░█▄▄▄▄▄▄▄█░▌▐░█▄▄▄▄▄▄▄█░▌▐░▌     ▐░▐░▌     ▐░▌║
║   ▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░░░░░░░░░░░▌▐░▌      ▐░░▌      ▐░▌║
║    ▀▀▀▀▀▀▀▀▀▀▀  ▀▀▀▀▀▀▀▀▀▀▀  ▀▀▀▀▀▀▀▀▀▀▀  ▀        ▀▀        ▀ ║
║                                                              ║
║              AETHELIAN DECRYPTION TERMINAL v2.4             ║
║                    CLASSIFIED ACCESS ONLY                   ║
║                                                              ║
╚══════════════════════════════════════════════════════════════╝

"""
        print(splash)
        self._typewriter_print("Initializing neural interface...", 0.02)
        self._loading_animation("Connecting to SIGNAL network", 1.0)
        print("\n✓ CONNECTION ESTABLISHED")
        input("\nPress ENTER to continue...")
    
    def show_main_menu(self):
        """Display main terminal menu"""
        while True:
            os.system('clear' if os.name == 'posix' else 'cls')
            
            print("╔" + "═"*58 + "╗")
            print("║" + " AETHELIAN DECRYPTION TERMINAL - MAIN INTERFACE ".center(58) + "║")
            print("╠" + "═"*58 + "╣")
            print(f"║ Discovered Passwords: {len(self.discovered_passwords):<20} Hack Level: {self.hacking_progress:<6} ║")
            print(f"║ Available Documents: {len(self.available_documents):<21} Status: {'ONLINE':<7} ║")
            print("╠" + "═"*58 + "╣")
            print("║                                                          ║")
            print("║  [1] 📁 Browse Classified Documents                      ║")
            print("║  [2] 🔓 Attempt Document Decryption                     ║")
            print("║  [3] 🔐 Encrypt New Document                            ║")
            print("║  [4] 🎯 Password Discovery Protocol                     ║")
            print("║  [5] 📡 Network Status & Help                           ║")
            print("║  [0] 🚪 Exit Terminal                                   ║")
            print("║                                                          ║")
            print("╚" + "═"*58 + "╝")
            
            choice = input("\nSelect option [0-5]: ").strip()
            
            if choice == "1":
                self.browse_documents()
            elif choice == "2":
                self.decrypt_document()
            elif choice == "3":
                self.encrypt_document()
            elif choice == "4":
                self.password_discovery()
            elif choice == "5":
                self.show_help()
            elif choice == "0":
                self._typewriter_print("Disconnecting from SIGNAL network...")
                self._loading_animation("Clearing traces", 1.0)
                print("Connection terminated.")
                break
            else:
                print("Invalid option. Press ENTER to continue...")
                input()
    
    def browse_documents(self):
        """Browse available classified documents"""
        os.system('clear' if os.name == 'posix' else 'cls')
        print("📁 CLASSIFIED DOCUMENT ARCHIVE\n")
        
        if not self.available_documents:
            print("❌ No classified documents found in archive.")
            print("   Ensure you're running from the correct directory.")
            input("\nPress ENTER to continue...")
            return
        
        for i, (filename, info) in enumerate(self.available_documents.items(), 1):
            status = "🔓 ACCESSIBLE" if info['passwords'] <= len(self.discovered_passwords) else "🔒 ENCRYPTED"
            print(f"[{i}] {filename}")
            print(f"    Encryption: SEED-{info['seed']} CURVE-{info['curve']} PASSWORDS-{info['passwords']}")
            print(f"    Status: {status}")
            print()
        
        choice = input("Enter document number to view, or ENTER to return: ").strip()
        
        if choice.isdigit():
            doc_idx = int(choice) - 1
            if 0 <= doc_idx < len(self.available_documents):
                doc_name = list(self.available_documents.keys())[doc_idx]
                self.view_document(doc_name)
    
    def view_document(self, doc_name: str):
        """View a specific document"""
        doc = self.available_documents[doc_name]
        
        os.system('clear' if os.name == 'posix' else 'cls')
        print(f"📄 VIEWING: {doc_name}\n")
        
        # Extract encrypted content
        content = doc['content']
        if "---\n" in content:
            parts = content.split("---\n")
            if len(parts) >= 3:
                encrypted_text = parts[2].split("\n---\n")[0] if "\n---\n" in parts[2] else parts[2]
            else:
                encrypted_text = parts[-1]
        else:
            encrypted_text = content
        
        # Show header info
        print(f"🔐 Encryption Parameters:")
        print(f"   Seed: {doc['seed']}")
        print(f"   Curve: {doc['curve']}")
        print(f"   Required Passwords: {doc['passwords']}")
        print(f"   Your Passwords: {len(self.discovered_passwords)}")
        print()
        
        if len(self.discovered_passwords) == 0:
            print("❌ No passwords available. Document appears as encrypted data:")
            print("─" * 60)
            # Show first few lines of encrypted text
            lines = encrypted_text.strip().split('\n')[:5]
            for line in lines:
                print(line[:80] + ("..." if len(line) > 80 else ""))
            print("─" * 60)
        else:
            print(f"🔓 Attempting decryption with {len(self.discovered_passwords)} password(s)...")
            
            # Perform decryption
            decryptor = DocumentDecryptor(doc['seed'], doc['curve'], doc['passwords'])
            decrypted_text, stats = decryptor.decrypt_with_passwords(
                encrypted_text, self.discovered_passwords, self.word_list
            )
            
            print(f"📊 Decryption Success: {stats['decryption_percentage']}%")
            print("─" * 60)
            
            # Show decrypted content
            lines = decrypted_text.strip().split('\n')
            for i, line in enumerate(lines):
                if i > 20:  # Limit display
                    print("... (truncated, full content available with complete decryption)")
                    break
                print(line)
            
            print("─" * 60)
        
        input("\nPress ENTER to continue...")
    
    def decrypt_document(self):
        """Interactive document decryption"""
        os.system('clear' if os.name == 'posix' else 'cls')
        print("🔓 DOCUMENT DECRYPTION PROTOCOL\n")
        
        if not self.discovered_passwords:
            print("❌ No passwords discovered yet.")
            print("   Use option [4] to attempt password discovery.")
            input("\nPress ENTER to continue...")
            return
        
        print(f"Available passwords: {', '.join(self.discovered_passwords)}")
        print("Available documents:")
        
        for i, doc_name in enumerate(self.available_documents.keys(), 1):
            print(f"  [{i}] {doc_name}")
        
        choice = input("\nSelect document number: ").strip()
        
        if choice.isdigit():
            doc_idx = int(choice) - 1
            if 0 <= doc_idx < len(self.available_documents):
                doc_name = list(self.available_documents.keys())[doc_idx]
                self.view_document(doc_name)
    
    def encrypt_document(self):
        """Interactive document encryption"""
        os.system('clear' if os.name == 'posix' else 'cls')
        print("🔐 DOCUMENT ENCRYPTION PROTOCOL\n")
        
        print("Enter document content (end with '###' on a new line):")
        lines = []
        while True:
            line = input()
            if line.strip() == "###":
                break
            lines.append(line)
        
        content = '\n'.join(lines)
        if not content.strip():
            print("❌ No content provided.")
            input("Press ENTER to continue...")
            return
        
        # Get encryption parameters
        seed = input("Enter 4-digit seed: ").strip()
        curve = input("Enter 4-digit curve: ").strip()
        
        passwords = []
        print("Enter passwords (empty line to finish):")
        while True:
            password = input(f"Password {len(passwords) + 1}: ").strip()
            if not password:
                break
            passwords.append(password)
        
        if not passwords:
            print("❌ No passwords provided.")
            input("Press ENTER to continue...")
            return
        
        try:
            # Perform encryption
            encryptor = DocumentEncryptor(seed, curve, passwords)
            encrypted_text, metadata = encryptor.encrypt_document(content)
            
            print(f"\n✓ ENCRYPTION COMPLETE")
            print(f"Parameters: SEED-{seed} CURVE-{curve} PASSWORDS-{len(passwords)}")
            print(f"Word count: {metadata['word_count']}")
            print("\n" + "─" * 60)
            print(encrypted_text)
            print("─" * 60)
            
            # Offer to save
            save = input("\nSave to file? (y/n): ").strip().lower()
            if save == 'y':
                filename = input("Enter filename: ").strip()
                if filename:
                    with open(filename, 'w') as f:
                        f.write(encrypted_text)
                    print(f"✓ Saved to {filename}")
        
        except Exception as e:
            print(f"❌ Encryption failed: {e}")
        
        input("\nPress ENTER to continue...")
    
    def password_discovery(self):
        """Password discovery minigame"""
        os.system('clear' if os.name == 'posix' else 'cls')
        print("🎯 PASSWORD DISCOVERY PROTOCOL\n")
        
        known_passwords = ["aethelia", "shadowhands", "voidcrown"]
        undiscovered = [p for p in known_passwords if p not in self.discovered_passwords]
        
        if not undiscovered:
            print("✓ All known passwords discovered!")
            print("Available passwords:", ", ".join(self.discovered_passwords))
            input("\nPress ENTER to continue...")
            return
        
        print("Scanning for encrypted password fragments...")
        self._loading_animation("Neural pattern analysis", 2.0)
        
        target_password = undiscovered[0]  # Always try to discover in order
        
        print(f"\n🧠 NEURAL INTERFACE CHALLENGE")
        print("Complete the hacking sequence to extract password data...")
        
        if self._hack_simulation(difficulty=len(self.discovered_passwords) + 2):
            print(f"\n🎉 PASSWORD DISCOVERED: '{target_password}'")
            self.discovered_passwords.append(target_password)
            print(f"Total passwords: {len(self.discovered_passwords)}/{len(known_passwords)}")
            
            if len(self.discovered_passwords) == len(known_passwords):
                print("\n🏆 ALL PASSWORDS DISCOVERED!")
                print("You now have full access to the classified archives!")
        
        input("\nPress ENTER to continue...")
    
    def show_help(self):
        """Show help and network status"""
        os.system('clear' if os.name == 'posix' else 'cls')
        print("📡 NETWORK STATUS & HELP\n")
        
        print("🔧 SYSTEM STATUS:")
        print(f"   Terminal Version: 2.4.1")
        print(f"   Encryption Protocol: Aethelian Progressive")
        print(f"   Network Connection: {'🟢 STABLE' if self.available_documents else '🔴 OFFLINE'}")
        print(f"   Word Database: {'🟢 LOADED' if self.word_list else '🔴 MISSING'}")
        print()
        
        print("📚 HELP:")
        print("   • Browse documents to see available classified files")
        print("   • Use password discovery to unlock decryption keys")
        print("   • Each password unlocks more content progressively")
        print("   • Encrypt your own documents with custom parameters")
        print("   • Higher hack levels unlock more challenging content")
        print()
        
        print("🎮 GAME MECHANICS:")
        print("   • Find passwords: aethelia, shadowhands, voidcrown")
        print("   • Each password unlocks ~33% more document content")
        print("   • Complete neural challenges to discover passwords")
        print("   • Experiment with encryption on your own text")
        print()
        
        print("📁 DOCUMENT LOCATIONS:")
        print("   • docs/SIGNAL/classified/ - Encrypted documents")
        print("   • tools/ - Encryption/decryption utilities")
        print()
        
        input("Press ENTER to continue...")

def main():
    try:
        terminal = AethelianTerminal()
        terminal.show_splash_screen()
        terminal.show_main_menu()
    except KeyboardInterrupt:
        print("\n\n⚠️  Terminal session interrupted.")
        print("Connection terminated.")
    except Exception as e:
        print(f"\n❌ Terminal error: {e}")
        print("Please ensure you're running from the project root directory.")

if __name__ == "__main__":
    main()
