#!/usr/bin/env python3
"""
SIGNAL Document Decryption Script
Progressive word-level decryption using discovered passwords
"""

import hashlib
import json
import re
import random
from typing import List, Dict, Set, Tuple, Optional

class DocumentDecryptor:
    def __init__(self, seed: str, curve: str, total_passwords: int):
        """
        Initialize decryptor with document parameters
        
        Args:
            seed: 4-digit string from document header
            curve: 4-digit string from document header  
            total_passwords: Total number of passwords used in encryption
        """
        self.seed = seed
        self.curve = curve
        self.total_passwords = total_passwords
        self.random = random.Random(int(seed))
        
        # Parse curve into percentages
        curve_digits = [int(d) for d in curve]
        self.curve_percentages = self._calculate_curve_percentages(curve_digits)
        
    def _calculate_curve_percentages(self, curve_digits: List[int]) -> List[float]:
        """Convert curve digits to cumulative percentages"""
        total = sum(curve_digits)
        if total == 0:
            return [1.0 / len(curve_digits)] * len(curve_digits)
        
        percentages = []
        cumulative = 0
        for digit in curve_digits:
            cumulative += digit / total
            percentages.append(cumulative)
        
        return percentages
    
    def _find_encrypted_words(self, text: str) -> List[Dict]:
        """Find all encrypted word tokens in text"""
        encrypted_words = []
        for match in re.finditer(r'\[([A-F0-9]{8})\]', text):
            encrypted_words.append({
                'token': match.group(0),
                'hash': match.group(1),
                'start': match.start(),
                'end': match.end()
            })
        return encrypted_words
    
    def _reconstruct_word_assignments(self, total_words: int) -> Dict[int, List[int]]:
        """Reconstruct which words were assigned to which password levels"""
        word_assignments = {}
        for i in range(self.total_passwords):
            word_assignments[i] = []
        
        # Recreate the same random assignment used during encryption
        word_indices = list(range(total_words))
        self.random.shuffle(word_indices)
        
        # Assign words based on curve percentages
        for i, word_idx in enumerate(word_indices):
            position_ratio = i / len(word_indices)
            
            password_level = 0
            for level, threshold in enumerate(self.curve_percentages):
                if position_ratio <= threshold:
                    password_level = level
                    break
                password_level = len(self.curve_percentages) - 1
            
            password_level = min(password_level, self.total_passwords - 1)
            word_assignments[password_level].append(word_idx)
        
        return word_assignments
    
    def _generate_encrypted_word(self, word: str, password: str) -> str:
        """Generate encrypted version to match against tokens"""
        hash_input = f"{word}_{password}_{self.seed}"
        word_hash = hashlib.md5(hash_input.encode()).hexdigest()[:8]
        return f"[{word_hash.upper()}]"
    
    def decrypt_with_passwords(self, encrypted_text: str, passwords: List[str], 
                             word_list: List[str]) -> Tuple[str, Dict]:
        """
        Decrypt text using available passwords and word list
        
        Args:
            encrypted_text: Text containing encrypted tokens
            passwords: List of discovered passwords  
            word_list: List of possible words for decryption attempts
            
        Returns:
            Partially/fully decrypted text and decryption statistics
        """
        encrypted_words = self._find_encrypted_words(encrypted_text)
        
        # Create reverse mapping of encrypted tokens to possible words
        decryption_map = {}
        successful_decryptions = 0
        
        for password in passwords:
            for word in word_list:
                encrypted_token = self._generate_encrypted_word(word, password)
                # Check if this encrypted token exists in the document
                for enc_word in encrypted_words:
                    if enc_word['token'] == encrypted_token:
                        decryption_map[enc_word['token']] = word
                        successful_decryptions += 1
        
        # Apply decryptions to text
        decrypted_text = encrypted_text
        for token, word in decryption_map.items():
            decrypted_text = decrypted_text.replace(token, word)
        
        # Calculate statistics
        total_encrypted = len(encrypted_words)
        remaining_encrypted = len(re.findall(r'\[[A-F0-9]{8}\]', decrypted_text))
        decryption_percentage = ((total_encrypted - remaining_encrypted) / total_encrypted * 100) if total_encrypted > 0 else 100
        
        stats = {
            'total_encrypted_words': total_encrypted,
            'successfully_decrypted': successful_decryptions,
            'remaining_encrypted': remaining_encrypted,
            'decryption_percentage': round(decryption_percentage, 1),
            'passwords_used': len(passwords),
            'passwords_total': self.total_passwords
        }
        
        return decrypted_text, stats

def load_word_list(filename: Optional[str] = None) -> List[str]:
    """Load word list for decryption attempts"""
    if filename:
        with open(filename, 'r') as f:
            words = []
            for line in f:
                line = line.strip().lower()
                # Skip empty lines and comments
                if line and not line.startswith('#'):
                    words.append(line)
            return words
    
    # Default word list for testing
    return [
        'black', 'armada', 'represents', 'fundamental', 'threat', 'empire', 'civilization',
        'they', 'operate', 'exploiting', 'gaps', 'conflicts', 'between', 'four', 'empires',
        'growing', 'stronger', 'while', 'compete', 'against', 'each', 'other',
        'recent', 'monument', 'salvage', 'operations', 'uncovered', 'evidence',
        'operatives', 'possess', 'complete', 'architectural', 'schematics', 'advanced',
        'signal', 'decoding', 'capabilities', 'the', 'and', 'a', 'to', 'of', 'in', 'is',
        'have', 'that', 'our', 'with', 'for', 'on', 'are', 'by', 'we', 'an'
    ]

def decrypt_with_passwords(file_path: str, seed: str, curve: str, total_passwords: int, 
                          passwords: List[str], wordlist_path: str) -> str:
    """
    Wrapper function for CLI compatibility
    
    Args:
        file_path: Path to encrypted document
        seed: 4-digit seed string
        curve: 4-digit curve string  
        total_passwords: Total number of passwords used in encryption
        passwords: List of passwords to try
        wordlist_path: Path to wordlist file
        
    Returns:
        Decrypted text content
    """
    # Load encrypted document
    with open(file_path, 'r') as f:
        encrypted_text = f.read()
    
    # Load word list
    word_list = load_word_list(wordlist_path)
    
    # Create decryptor and perform decryption
    decryptor = DocumentDecryptor(seed, curve, total_passwords)
    decrypted_text, stats = decryptor.decrypt_with_passwords(encrypted_text, passwords, word_list)
    
    return decrypted_text

def main():
    import sys
    
    if len(sys.argv) < 4:
        print("Usage: python decrypt_document.py <seed> <curve> <total_passwords> [encrypted_file] [word_list_file]")
        print("Example: python decrypt_document.py 1337 2468 3")
        return
    
    seed = sys.argv[1]
    curve = sys.argv[2] 
    total_passwords = int(sys.argv[3])
    
    # Sample encrypted text for testing
    encrypted_text = """
    The [5F2A8B1C] [8D4E9A2F] represents a [C3B7E4D1] [9A5F3C8B] to [2E8F4A9D] [7B3C9E4F]. 
    [1F4D8A2C] [6E9B3F5A] by [4A8D2F7C] the [3C7E1B9F] and [8F5A4D3C] [9B2E7F4A] our [5D8A3F2E] [2C9F4B7A], 
    [7A4F8D2C] [3E9B5A8F] [1F7C4A9D] we [6D2F8A4C] [9A5E3F7B] [4F8C2A9D] [7E3B1F5A].
    
    [8A3F5D9C] [2F7E4A8D] [5C9A3F7E] [4D8F2A5C] have [1A7F4D9C] [6F3E8A2D] that [9C5A8F3E] [7D2F4A9C] 
    [3A8E5F7D] possess [5F9C2A8D] [4E7A3F9C] [8D5F2A7E] and [6A3F9D4C] [2F8E5A7D] 
    [9C4A8F3E] [7F2D5A9C].
    """
    
    # Test with progressive password discovery
    passwords_discovered = ["aethelia"]  # Start with first password
    word_list = load_word_list()
    
    decryptor = DocumentDecryptor(seed, curve, total_passwords)
    
    print(f"=== DECRYPTION ANALYSIS ===")
    print(f"Document Seed: {seed}")
    print(f"Document Curve: {curve}")
    print(f"Total Passwords: {total_passwords}")
    print()
    
    # Show progressive decryption
    for i in range(1, len(passwords_discovered) + 1):
        current_passwords = passwords_discovered[:i]
        decrypted_text, stats = decryptor.decrypt_with_passwords(
            encrypted_text, current_passwords, word_list
        )
        
        print(f"=== WITH {i} PASSWORD(S): {current_passwords} ===")
        print(f"Decryption: {stats['decryption_percentage']}% complete")
        print(f"Words decrypted: {stats['successfully_decrypted']}/{stats['total_encrypted_words']}")
        print()
        print("DECRYPTED TEXT:")
        print(decrypted_text[:200] + "..." if len(decrypted_text) > 200 else decrypted_text)
        print()

if __name__ == "__main__":
    main()
