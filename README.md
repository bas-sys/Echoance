# Echoance - The Emotional Stream

> A narrative visual novel game built with C++ and SFML

---

## Overview

**Echoance** is a choice-driven narrative game where players dive into the "Emotional Stream" - a mysterious system that lets them experience memories of their ancestors across different time periods. Every choice shapes the story's outcome, leading to one of four distinct endings.

---

## Story Synopsis

You wake up in Nexford Research Facility, connected to a machine called the Emotional Stream. As Subject 12, you're thrown into memory jumps across time:

1. **The Scholar (1800s)** - Meet Charlotte Nexford in an endless library
2. **The Rebel (1930s)** - Fight alongside Jace Nexford in a war-torn city  
3. **The Scientist (Modern Era)** - Confront Dr. Damon Nexford and The Echo

Your final choice determines whether you break free, continue the cycle, merge with the echo, or collapse entirely.

---

## Project Structure

```
Echoance/
├── main.cpp                    # Entry point
├── echoance.exe                # Compiled game
│
├── include/
│   ├── GameEngine.h            # Main game engine class
│   ├── Memory.h                # Memory, Character, Echo structs
│   ├── MemoryManager.h         # Story data manager
│   ├── Choice.h                # Choice struct with sub-choices
│   ├── NarrativeUI.h           # Polished UI components
│   └── nlohmann/json.hpp       # JSON parser library
│
├── src/
│   ├── story.json              # All narrative content
│   └── engine/
│       ├── GameEngine.cpp      # Game logic implementation
│       └── MemoryManager.cpp   # JSON loading logic
│
└── SFML-2.6.1/                 # SFML library files
```

---

## Core Components

### GameEngine

The heart of the game. Handles:
- Game state management (prologue, memories, endings)
- Event handling (keyboard + mouse input)
- Rendering (background, particles, UI)
- Scene transitions with fade effects

**Key States:**
| State | Description |
|-------|-------------|
| `PROLOGUE_NARRATION` | Opening story text |
| `PROLOGUE_CHOICES` | First choices in the lab |
| `MEMORY_NARRATION` | Memory jump story text |
| `MEMORY_CHOICES` | Choices during memory |
| `SHOW_OUTCOME` | Result of a choice |
| `ENDING` | Final ending narration |
| `GAME_OVER` | Exit prompt |

### MemoryManager

Loads all story content from `story.json`:
- Prologue (narration, thoughts, system dialogue, choices)
- Memory jumps (narration, NPCs, Echo, choices)
- Endings (4 possible conclusions)

### NarrativeUI

Polished UI system with:
- **DynamicTextBox** - Auto-sizing dialogue box with typewriter animation
- **ChoiceCard** - Rounded choice buttons with glow effects
- **ChoicePanel** - Stacked choice navigation

---

## Controls

| Key | Action |
|-----|--------|
| **Space** | Skip typewriter / Advance dialogue |
| **Enter** | Confirm selected choice |
| **↑ / ↓** | Navigate choices |
| **Mouse Hover** | Highlight choice |
| **Mouse Click** | Select and confirm choice instantly |
| **ESC** | Exit game |

---

## Story Content (story.json)

All narrative content is externalized to `story.json`:

```json
{
  "game_title": "Echoance",
  "subtitle": "The Emotional Stream",
  "prologue": { ... },
  "memory_jumps": [ ... ],
  "endings": [ ... ]
}
```

### Choice Structure
```json
{
  "id": "break_cycle",
  "text": "Break the cycle - choose freedom",
  "outcome": "You rip the device out...",
  "next_memory_id": ""  // Empty = triggers ending lookup
}
```

### Endings
| ID | Title |
|----|-------|
| `break_cycle` | Break the Cycle (Freedom) |
| `continue_cycle` | Continue the Cycle (Legacy) |
| `merge_echo` | Merge with the Echo (Transcendence) |
| `panic` | Collapse (Failure) |

---

## Visual Features

### Color Palette
- **Background:** Deep purple/navy (#0F0C1E)
- **Accent Cyan:** #64DCFF
- **Accent Pink:** #FF78B4
- **Text:** White with cyan highlights

### Effects
- Floating particle system (cyan + pink orbs)
- Fade transitions between scenes
- Pulsing glow effects on UI
- Scale animation on choice hover (1.01x - 1.02x)
- Typewriter text animation

### UI Polish
- Rounded corners on all elements
- Multi-layer glow (outer + inner)
- Animated shimmer line on dialogue box
- Semi-transparent backgrounds (80% opacity)

---

## Building the Game

### Requirements
- C++ compiler (g++ or MSVC)
- SFML 2.6.1

### Compile Command
```bash
g++ -o echoance.exe main.cpp src/engine/GameEngine.cpp src/engine/MemoryManager.cpp \
    -I./include -I./SFML-2.6.1/include \
    -L./SFML-2.6.1/lib \
    -lsfml-graphics -lsfml-window -lsfml-system
```

### Run
```bash
./echoance.exe
```

---

## Game Flow

```
┌─────────────┐
│  PROLOGUE   │
│  Narration  │
└──────┬──────┘
       ▼
┌─────────────┐
│  Prologue   │
│   Choices   │
└──────┬──────┘
       ▼
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Memory 1   │────▶│  Memory 2   │────▶│  Memory 3   │
│  Scholar    │     │   Rebel     │     │  Scientist  │
└─────────────┘     └─────────────┘     └──────┬──────┘
                                               ▼
                                   ┌───────────────────┐
                                   │   FINAL CHOICE    │
                                   │ (4 ending paths)  │
                                   └─────────┬─────────┘
                                             ▼
                          ┌──────────────────┴──────────────────┐
                          │                                     │
                    ┌─────┴─────┐  ┌─────────┐  ┌───────┐  ┌────┴───┐
                    │  BREAK    │  │CONTINUE │  │ MERGE │  │ PANIC  │
                    │  CYCLE    │  │  CYCLE  │  │ ECHO  │  │        │
                    └───────────┘  └─────────┘  └───────┘  └────────┘
```

---

## Technical Details

### Window Configuration
- **Size:** 1400 x 850 pixels
- **FPS:** 60 (locked)
- **Font:** Arial (Windows system font)

### Dual UI System
The game has two UI systems that run in parallel:
1. **Legacy UI** (`DialogueBox`, `ChoiceSelector`) - Original implementation
2. **Polished UI** (`DynamicTextBox`, `ChoicePanel`) - Enhanced visuals

Toggle between them with `usePolishedUI` flag in `GameEngine.h`.

### Input Priority
1. Mouse hover updates selection index
2. Keyboard arrows override mouse selection
3. Both click and Enter confirm choices

---

## Troubleshooting

### Square Characters in Text
**Cause:** Special Unicode characters (em-dash, ellipsis) not supported by Arial
**Fix:** Use standard ASCII alternatives (-- instead of —, ... instead of …)

### Game Freezes After Chapter
**Cause:** ID mismatch between choice `next_memory_id` and actual memory `id`
**Fix:** Ensure all IDs in story.json use consistent naming (underscores)

### Ending Not Triggering
**Cause:** Choice ID doesn't match ending ID in story.json
**Fix:** Final chapter choice IDs must exactly match ending IDs

---

## Credits

- **Engine:** SFML 2.6.1
- **JSON Parser:** nlohmann/json
- **Development:** Built with C++17

---

*Echoance - Every choice echoes through time.*
