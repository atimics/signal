#!/usr/bin/env python3
"""
SIGNAL Document Encryption Script
Progressive word-level encryption with seed, curve, and multiple passwords
"""

import random
import hashlib
import json
import re
from typing import List, Dict, Tuple

class DocumentEncryptor:
    def __init__(self, seed: str, curve: str, passwords: List[str]):
        """
        Initialize encryptor with 4-digit seed, 4-digit curve, and password list
        
        Args:
            seed: 4-digit string (e.g., "1337")
            curve: 4-digit string defining encryption curve (e.g., "2468")
            passwords: List of passwords in unlock order
        """
        self.seed = seed
        self.curve = curve
        self.passwords = passwords
        self.random = random.Random(int(seed))
        
        # Parse curve into percentages
        curve_digits = [int(d) for d in curve]
        self.curve_percentages = self._calculate_curve_percentages(curve_digits)
        
    def _calculate_curve_percentages(self, curve_digits: List[int]) -> List[float]:
        """Convert curve digits to cumulative percentages"""
        # Normalize curve digits to percentages
        total = sum(curve_digits)
        if total == 0:
            return [1.0 / len(curve_digits)] * len(curve_digits)
        
        percentages = []
        cumulative = 0
        for digit in curve_digits:
            cumulative += digit / total
            percentages.append(cumulative)
        
        return percentages
    
    def _extract_words(self, text: str) -> List[Tuple[str, int, int]]:
        """Extract words with their positions"""
        words = []
        for match in re.finditer(r'\b[A-Za-z]+\b', text):
            words.append((match.group(), match.start(), match.end()))
        return words
    
    def _assign_words_to_passwords(self, words: List[Tuple[str, int, int]]) -> Dict[int, List[int]]:
        """Assign word indices to password levels based on curve"""
        word_assignments = {}
        num_passwords = len(self.passwords)
        
        # Initialize password assignments
        for i in range(num_passwords):
            word_assignments[i] = []
        
        # Shuffle words deterministically
        word_indices = list(range(len(words)))
        self.random.shuffle(word_indices)
        
        # Assign words based on curve percentages
        for i, word_idx in enumerate(word_indices):
            # Determine which password level this word belongs to
            position_ratio = i / len(word_indices)
            
            password_level = 0
            for level, threshold in enumerate(self.curve_percentages):
                if position_ratio <= threshold:
                    password_level = level
                    break
                password_level = len(self.curve_percentages) - 1
            
            # Ensure we don't exceed password count
            password_level = min(password_level, num_passwords - 1)
            word_assignments[password_level].append(word_idx)
        
        return word_assignments
    
    def _generate_encrypted_word(self, word: str, password: str) -> str:
        """Generate encrypted version of word using password"""
        # Simple but deterministic encryption
        hash_input = f"{word}_{password}_{self.seed}"
        word_hash = hashlib.md5(hash_input.encode()).hexdigest()[:8]
        return f"[{word_hash.upper()}]"
    
    def encrypt_document(self, text: str) -> Tuple[str, Dict]:
        """Encrypt document and return encrypted text + metadata"""
        words = self._extract_words(text)
        word_assignments = self._assign_words_to_passwords(words)
        
        # Create encryption mapping
        encryption_map = {}
        for password_level, word_indices in word_assignments.items():
            password = self.passwords[password_level]
            for word_idx in word_indices:
                word, start, end = words[word_idx]
                encrypted_word = self._generate_encrypted_word(word, password)
                encryption_map[word_idx] = {
                    'original': word,
                    'encrypted': encrypted_word,
                    'password_level': password_level,
                    'start': start,
                    'end': end
                }
        
        # Apply encryption to text
        encrypted_text = text
        offset = 0
        
        # Sort by position to maintain text integrity
        sorted_words = sorted(words, key=lambda x: x[1])
        for i, (word, start, end) in enumerate(sorted_words):
            if i in encryption_map:
                encrypted_word = encryption_map[i]['encrypted']
                # Adjust positions for previous replacements
                adj_start = start + offset
                adj_end = end + offset
                
                encrypted_text = (encrypted_text[:adj_start] + 
                                encrypted_word + 
                                encrypted_text[adj_end:])
                
                offset += len(encrypted_word) - len(word)
        
        # Create metadata
        metadata = {
            'seed': self.seed,
            'curve': self.curve,
            'passwords': len(self.passwords),
            'word_count': len(words),
            'encryption_stats': {
                f'level_{i}': len(indices) for i, indices in word_assignments.items()
            }
        }
        
        return encrypted_text, metadata

def encrypt_document_with_params(input_file: str, output_file: str, seed: str, curve: str,
                                password_count: int, passwords: List[str], wordlist_path: str) -> None:
    """
    Wrapper function for CLI compatibility
    
    Args:
        input_file: Path to input document to encrypt
        output_file: Path for encrypted output
        seed: 4-digit seed string
        curve: 4-digit curve string
        password_count: Number of passwords (should match len(passwords))
        passwords: List of passwords for encryption
        wordlist_path: Path to wordlist file (not used in encryption but kept for compatibility)
    """
    # Load input document
    with open(input_file, 'r') as f:
        original_text = f.read()
    
    # Create encryptor and encrypt
    encryptor = DocumentEncryptor(seed, curve, passwords)
    encrypted_text, metadata = encryptor.encrypt_document(original_text)
    
    # Create document header with metadata
    header = f"""---
CLASSIFICATION: CLASSIFIED - EYES ONLY
SIGNAL PROTOCOL: AETHELIAN NETWORK
ENCRYPTION: SEED-{seed} CURVE-{curve} PASSWORDS-{password_count}
ACCESS: PROGRESSIVE DECRYPTION REQUIRED
---

{encrypted_text}"""
    
    # Write encrypted document
    with open(output_file, 'w') as f:
        f.write(header)
    
    print(f"Document encrypted and saved to: {output_file}")

def main():
    # Example usage
    sample_text = """
    The Black Armada represents a fundamental threat to empire civilization. 
    They operate by exploiting the gaps and conflicts between our four empires, 
    growing stronger while we compete against each other.
    
    Recent Monument salvage operations have uncovered evidence that Black Armada 
    operatives possess complete architectural schematics and advanced signal 
    decoding capabilities.
    """
    
    # Configuration
    seed = "1337"
    curve = "2468"  # Curve shape: 20%, 40%, 60%, 80% thresholds
    passwords = ["aethelia", "shadowhands", "voidcrown"]
    
    # Encrypt
    encryptor = DocumentEncryptor(seed, curve, passwords)
    encrypted_text, metadata = encryptor.encrypt_document(sample_text)
    
    # Output results
    print("=== ENCRYPTION RESULTS ===")
    print(f"Seed: {seed}")
    print(f"Curve: {curve}")
    print(f"Passwords: {passwords}")
    print()
    print("=== ENCRYPTED TEXT ===")
    print(encrypted_text)
    print()
    print("=== METADATA ===")
    print(json.dumps(metadata, indent=2))

if __name__ == "__main__":
    main()
