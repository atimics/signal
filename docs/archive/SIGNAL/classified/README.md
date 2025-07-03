# SIGNAL Classified Documents & CLI Minigame

This folder contains encrypted intelligence documents that players can discover and decrypt using the SIGNAL CLI minigame.

## 🎮 SIGNAL CLI Minigame

Access the interactive classified document interface with:
```bash
./signal
```

The CLI provides an immersive terminal interface for:
- 📂 Browsing classified documents
- 🔍 Viewing document metadata and encryption status
- 🔓 Attempting progressive decryption with discovered passwords
- 📝 Encrypting new documents (for developers)

### Features
- **Interactive Interface**: Typewriter effects, colored output, immersive session tracking
- **Progressive Decryption**: Each password unlocks more document content
- **Real-time Feedback**: Shows decryption progress and success rates
- **Session Persistence**: Tracks discovered passwords during gameplay session

## How Progressive Decryption Works

1. **Document Discovery**: Players find encrypted files in the repository
2. **Key Discovery**: Players must find decryption passwords through gameplay and lore exploration
3. **Progressive Unlock**: Each discovered password unlocks additional portions of the document
4. **Complete Intelligence**: Full document readability requires all passwords

## Available Documents

- `AETHELIAN_NETWORK_ENCRYPTED.md`: Archaeological secrets about the Monument (1 password required)
- `BLACK_ARMADA_ENCRYPTED.md`: Intelligence on the hidden fifth empire (3 passwords required)
- `NOVA_HEGEMONY_ENCRYPTED.md`: Intelligence assessment on the technocratic empire (1 password required)
- `CRIMSON_SYNDICATE_ENCRYPTED.md`: Criminal network analysis (2 passwords required)
- `GHOST_PROTOCOLS_ENCRYPTED.md`: Secret communication systems (1 password required)
- `MONUMENT_ARCHAEOLOGY_ENCRYPTED.md`: Deep archaeological findings (2 passwords required)
- `FTL_RESEARCH_ENCRYPTED.md`: Top secret faster-than-light research (3 passwords required)

## Known Passwords (Gameplay Spoilers)

🚨 **SPOILER WARNING** - For developers and testers only: aethelia

## Manual Decryption Usage

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
