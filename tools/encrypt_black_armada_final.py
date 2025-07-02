#!/usr/bin/env python3
"""
Encrypt Black Armada document for game deployment
"""

import sys
import os
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from encrypt_document import DocumentEncryptor
import json

def main():
    # Read the original document
    source_path = 'docs/SIGNAL/vision/lore/BLACK_ARMADA_CLASSIFIED.md'
    with open(source_path, 'r') as f:
        original_text = f.read()
    
    # Encryption parameters - these will be visible in the encrypted file
    seed = "5724"  # Visible on document
    curve = "1359"  # Visible on document  
    passwords = ["aethelia", "shadowhands", "voidcrown"]
    
    # Encrypt the document
    encryptor = DocumentEncryptor(seed, curve, passwords)
    encrypted_text, metadata = encryptor.encrypt_document(original_text)
    
    # Create the encrypted document with visible parameters
    encrypted_document = f"""# CLASSIFIED: BLACK ARMADA INTELLIGENCE BRIEFING
**ENCRYPTION STAMP**: SEED-{seed} CURVE-{curve} PASSWORDS-{len(passwords)}
**STATUS**: ENCRYPTED - PROGRESSIVE DECRYPTION REQUIRED
**CLASSIFICATION**: VOID BLACK - COMPARTMENTALIZED ACCESS ONLY
**DISTRIBUTION**: NEED-TO-KNOW BASIS - COUNTER-INTELLIGENCE DIVISION

**DECRYPTION PROGRESS**: 0/{len(passwords)} KEYS DISCOVERED
**ESTIMATED READABILITY**: {round(100/len(passwords), 1)}% PER KEY

**USAGE**: python tools/decrypt_document.py {seed} {curve} {len(passwords)} <discovered_passwords>

---

{encrypted_text}

---

**ENCRYPTION METADATA** (FOR TECHNICAL REFERENCE):
- Total Words: {metadata['word_count']}
- Distribution: {metadata['encryption_stats']}
- Decryption requires discovery of passwords through gameplay
"""

    # Write to classified folder
    output_path = 'docs/SIGNAL/classified/BLACK_ARMADA_ENCRYPTED.md'
    with open(output_path, 'w') as f:
        f.write(encrypted_document)
    
    # Create metadata file for developers
    metadata_path = 'docs/SIGNAL/classified/encryption_metadata.json'
    with open(metadata_path, 'w') as f:
        json.dump(metadata, f, indent=2)
    
    # Create README for the classified folder
    readme_content = """# SIGNAL Classified Documents

This folder contains encrypted intelligence documents that players can discover in the game source code but cannot read without finding the proper decryption keys through gameplay.

## How Progressive Decryption Works

1. **Document Discovery**: Players can find these encrypted files in the open source repository
2. **Key Discovery**: Players must find decryption passwords during gameplay
3. **Progressive Unlock**: Each discovered password unlocks additional portions of the document
4. **Complete Intelligence**: Full document readability requires all passwords

## Available Documents

- `BLACK_ARMADA_ENCRYPTED.md`: Intelligence on the hidden fifth empire
- `AETHELIAN_NETWORK_CLASSIFIED.md`: Archaeological secrets about the Monument

## Decryption Usage

```bash
# Example: decrypt with discovered passwords
python tools/decrypt_document.py 5724 1359 3 aethelia shadowhands
```

## For Developers

The encryption system uses:
- **Seed**: Deterministic random word selection
- **Curve**: Distribution of words across password levels  
- **Progressive**: Each password unlocks cumulative portions

See `tools/` folder for encryption and decryption scripts.
"""
    
    readme_path = 'docs/SIGNAL/classified/README.md'
    with open(readme_path, 'w') as f:
        f.write(readme_content)
    
    print(f"✅ Encrypted BLACK_ARMADA document created:")
    print(f"   📁 {output_path}")
    print(f"   📊 {metadata_path}")
    print(f"   📖 {readme_path}")
    print()
    print(f"🔐 Encryption Parameters:")
    print(f"   Seed: {seed}")
    print(f"   Curve: {curve}")
    print(f"   Passwords: {len(passwords)}")
    print(f"   Total Words: {metadata['word_count']}")
    print(f"   Distribution: {metadata['encryption_stats']}")

if __name__ == "__main__":
    main()
