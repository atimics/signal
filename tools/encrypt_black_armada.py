#!/usr/bin/env python3
"""
Apply encryption to Black Armada document
"""

from encrypt_document import DocumentEncryptor
import sys

def encrypt_black_armada():
    # Read the original document
    with open('../docs/SIGNAL/vision/lore/BLACK_ARMADA_CLASSIFIED.md', 'r') as f:
        original_text = f.read()
    
    # Encryption parameters
    seed = "5724"  # Seed stamped on document
    curve = "1359"  # Curve defining encryption distribution
    passwords = ["aethelia", "shadowhands", "voidcrown"]
    
    # Encrypt
    encryptor = DocumentEncryptor(seed, curve, passwords)
    encrypted_text, metadata = encryptor.encrypt_document(original_text)
    
    # Create encrypted version with document stamp
    document_header = f"""# CLASSIFIED: BLACK ARMADA INTELLIGENCE BRIEFING
**ENCRYPTION STAMP**: SEED-{seed} CURVE-{curve} PASSWORDS-{len(passwords)}
**ACCESS CODE**: [PROGRESSIVE_DECRYPTION_REQUIRED]
**CLASSIFICATION**: VOID BLACK - COMPARTMENTALIZED ACCESS ONLY
**DISTRIBUTION**: NEED-TO-KNOW BASIS - COUNTER-INTELLIGENCE DIVISION

**DECRYPTION STATUS**: {metadata['encryption_stats']}

---

"""
    
    # Write encrypted document
    with open('../docs/SIGNAL/vision/lore/BLACK_ARMADA_ENCRYPTED.md', 'w') as f:
        f.write(document_header + encrypted_text)
    
    # Write decryption metadata
    with open('../docs/SIGNAL/vision/lore/BLACK_ARMADA_METADATA.json', 'w') as f:
        import json
        json.dump(metadata, f, indent=2)
    
    print(f"Encrypted document created with:")
    print(f"  Seed: {seed}")
    print(f"  Curve: {curve}")
    print(f"  Passwords: {len(passwords)}")
    print(f"  Total words: {metadata['word_count']}")
    print(f"  Encryption distribution: {metadata['encryption_stats']}")

if __name__ == "__main__":
    encrypt_black_armada()
