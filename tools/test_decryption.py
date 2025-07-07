#!/usr/bin/env python3
"""
Test the encrypted documents to show progressive decryption
"""

import sys
import os
sys.path.append('tools')

from decrypt_document import DocumentDecryptor, load_word_list

def test_black_armada_decryption():
    print("=== TESTING BLACK ARMADA PROGRESSIVE DECRYPTION ===\n")
    
    # Read encrypted document
    with open('docs/SIGNAL/classified/BLACK_ARMADA_ENCRYPTED.md', 'r') as f:
        encrypted_content = f.read()
    
    # Extract just the encrypted text part (after the header)
    encrypted_text = encrypted_content.split('---\n')[2].split('\n---\n')[0]
    
    # Load word list
    word_list = load_word_list('tools/signal_wordlist.txt')
    
    # Initialize decryptor
    decryptor = DocumentDecryptor("5724", "1359", 3)
    
    # Test progressive decryption
    passwords_sequence = [
        [],
        ["aethelia"], 
        ["aethelia", "shadowhands"],
        ["aethelia", "shadowhands", "voidcrown"]
    ]
    
    for i, passwords in enumerate(passwords_sequence):
        print(f"--- DECRYPTION LEVEL {i}: {len(passwords)} PASSWORDS ---")
        if passwords:
            print(f"Using passwords: {passwords}")
        else:
            print("No passwords - raw encrypted text")
        
        if passwords:
            decrypted_text, stats = decryptor.decrypt_with_passwords(
                encrypted_text, passwords, word_list
            )
            print(f"Readability: {stats['decryption_percentage']}%")
            print(f"Words decrypted: {stats['successfully_decrypted']}/{stats['total_encrypted_words']}")
        else:
            decrypted_text = encrypted_text
            print("Readability: 0%")
        
        # Show first few lines
        lines = decrypted_text.strip().split('\n')[:3]
        for line in lines:
            print(f"  {line[:80]}{'...' if len(line) > 80 else ''}")
        print()

def test_aethelian_decryption():
    print("=== TESTING AETHELIAN NETWORK DECRYPTION ===\n")
    
    # Read encrypted document  
    with open('docs/SIGNAL/classified/AETHELIAN_NETWORK_ENCRYPTED.md', 'r') as f:
        encrypted_content = f.read()
    
    # Extract encrypted text
    encrypted_text = encrypted_content.split('---\n')[1]
    
    # Load word list
    word_list = load_word_list('tools/signal_wordlist.txt')
    
    # Initialize decryptor
    decryptor = DocumentDecryptor("9247", "6283", 1)
    
    print("--- WITHOUT PASSWORD ---")
    lines = encrypted_text.strip().split('\n')[:3]
    for line in lines:
        print(f"  {line[:80]}{'...' if len(line) > 80 else ''}")
    print()
    
    print("--- WITH PASSWORD: aethelia ---")
    decrypted_text, stats = decryptor.decrypt_with_passwords(
        encrypted_text, ["aethelia"], word_list
    )
    print(f"Readability: {stats['decryption_percentage']}%")
    lines = decrypted_text.strip().split('\n')[:3]
    for line in lines:
        print(f"  {line[:80]}{'...' if len(line) > 80 else ''}")
    print()

if __name__ == "__main__":
    test_black_armada_decryption()
    test_aethelian_decryption()
