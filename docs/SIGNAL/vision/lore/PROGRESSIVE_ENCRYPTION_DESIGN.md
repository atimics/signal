# SIGNAL: Progressive Word-Level Encryption System

**System**: Distributed Word Encryption with Progressive Decryption  
**Purpose**: Realistic intelligence security requiring multiple key discoveries  
**Implementation**: Random word selection with encrypted mapping tables

---

## Encryption Protocol

### Stage 1: Word Distribution
1. **Document Analysis**: Extract all significant words from source document
2. **Random Distribution**: Divide words into 3 roughly equal groups using cryptographic RNG
3. **Key Assignment**: Assign each group to keys "aethelia", "shadowhands", "voidcrown"
4. **Mapping Encryption**: Encrypt the word-to-key mapping table itself

### Stage 2: Progressive Encryption
- **Key 3 ("voidcrown")**: Encrypts 1/3 of words (randomly selected)
- **Key 2 ("shadowhands")**: Encrypts 1/3 of words + encrypted mapping from Key 3
- **Key 1 ("aethelia")**: Encrypts 1/3 of words + encrypted mappings from Keys 2&3

### Stage 3: Reading Requirements
- **No Keys**: Document appears as partial gibberish with 1/3 readable words
- **Key 1 Only**: Can decrypt 1/3 of encrypted words, 2/3 readable total
- **Keys 1+2**: Can decrypt 2/3 of encrypted words, all words readable except Key 3 set
- **All Keys**: Full document readable

---

## Example Implementation

### Original Text Section:
```
The Black Armada represents a fundamental threat to empire civilization. 
They operate by exploiting the gaps and conflicts between our four empires, 
growing stronger while we compete against each other.
```

### Word Distribution (Hidden Mapping):
- **Key 1 ("aethelia")**: [Black, fundamental, empire, operate, gaps, four, stronger, compete]
- **Key 2 ("shadowhands")**: [Armada, threat, civilization, exploiting, conflicts, empires, while, against]  
- **Key 3 ("voidcrown")**: [represents, They, the, and, between, our, growing, we, each, other]

### Progressive Encryption Result:

#### Stage 1 - No Keys (1/3 readable):
```
The [ENCRYPTED] [ENCRYPTED] a [ENCRYPTED] [ENCRYPTED] to [ENCRYPTED] [ENCRYPTED]. 
[ENCRYPTED] [ENCRYPTED] by [ENCRYPTED] the [ENCRYPTED] [ENCRYPTED] [ENCRYPTED] [ENCRYPTED] our [ENCRYPTED] [ENCRYPTED], 
[ENCRYPTED] [ENCRYPTED] [ENCRYPTED] [ENCRYPTED] [ENCRYPTED] [ENCRYPTED] [ENCRYPTED] [ENCRYPTED].
```

#### Stage 2 - Key "aethelia" (2/3 readable):
```
The [ENCRYPTED] represents a fundamental threat to empire [ENCRYPTED]. 
[ENCRYPTED] operate by exploiting the gaps [ENCRYPTED] [ENCRYPTED] [ENCRYPTED] our four [ENCRYPTED], 
[ENCRYPTED] stronger [ENCRYPTED] we compete [ENCRYPTED] [ENCRYPTED] [ENCRYPTED].
```

#### Stage 3 - Keys "aethelia" + "shadowhands" (all readable except Key 3):
```
The Armada represents a fundamental threat to empire civilization. 
[ENCRYPTED] operate by exploiting the gaps and conflicts between our four empires, 
[ENCRYPTED] stronger while we compete against [ENCRYPTED] [ENCRYPTED].
```

#### Stage 4 - All Keys (fully readable):
```
The Black Armada represents a fundamental threat to empire civilization. 
They operate by exploiting the gaps and conflicts between our four empires, 
growing stronger while we compete against each other.
```

---

## Cryptographic Implementation

### Word Selection Algorithm:
```c
// Pseudo-code for word distribution
uint32_t word_hash = hash_function(word + document_seed);
uint32_t key_assignment = word_hash % 3;  // 0="aethelia", 1="shadowhands", 2="voidcrown"
```

### Mapping Table Encryption:
```c
// Each key encrypts its own mapping plus previous mappings
typedef struct {
    char* encrypted_word;
    uint32_t original_position;
    uint32_t key_level;  // Which key can decrypt this word
} EncryptedWord;
```

### Progressive Decryption:
- **Key Discovery Order**: Players must find keys in sequence to decrypt mapping tables
- **Dependency Chain**: Later keys require earlier keys to decrypt their word mappings
- **Realistic Security**: Mimics real intelligence compartmentalization

---

## Implementation in Game Documents

This system would transform the classified documents into realistic intelligence puzzles where:

1. **Partial Information**: Players get tantalizing glimpses of truth with incomplete keys
2. **Progressive Revelation**: Each key dramatically improves document readability  
3. **Realistic Security**: Reflects how real intelligence agencies protect sensitive information
4. **Gameplay Integration**: Key discovery becomes meaningful progression mechanic

---

**Result**: Documents feel like real classified intelligence requiring proper security clearance levels to fully comprehend, while still providing partial information to drive player curiosity and investigation.
