# KÖKPAR: Wobbly Steppe

**Version 0.4 — Horse Anatomy & Kipchak Warrior**

A stylized, physics-driven 3D game celebrating the traditional Central Asian nomadic sport of **Kökpar** (Көкпар). Ride across the expansive steppe landscape, outmaneuver obstacles, snatch the serke (goat carcass), and deposit or fling it into the Kazanlyk (goal circle) to score!

---

## 🎮 Controls & Gameplay Guide

| Control | Action | Details |
| :--- | :--- | :--- |
| **W, A, S, D** / Arrows | **Ride & Steer** | Dynamic turning radius adapts to speed (nimble at walk, banking arcs at sprint) |
| **Shift** (Hold) | **Sprint Gallop** | Increases speed from Trot up to full Gallop (13.8 m/s) with dynamic FOV boost |
| **Space** | **Quick Brake** | Fast emergency halt to stop precisely beside the goat or before obstacles |
| **E** or **LMB** | **Grab / Fling Goat** | Pick up goat when in range; throw forward-right with horse momentum |
| **Mouse Look** | **360° Orbit Look** | Seamless orbital camera look with safety pitch clamping |
| **Mouse Scroll** | **Zoom Camera** | Zoom smoothly in and out from third-person to panoramic view |
| **Tab** | **Cursor Capture Toggle**| Release cursor to interact with window/menus; click to recapture |
| **Z / V** | **Reset Camera** | Instantly realigns third-person camera directly behind your horse |
| **C** | **Camera Modes** | Cycles between **Follow**, **Free Orbit**, and **Cinematic** views |
| **H / F1** | **Controls HUD** | Toggles the on-screen controls cheat-sheet panel |
| **R** | **Reset Game** | Restarts score, respawns goat and horse to initial state |
| **Esc** | **Exit** | Closes the application |

---

## ✨ Features in Version 0.4

### 🐎 Anatomical Equine Silhouette & Proportions
- **Sculpted Equine Silhouette**: Replaced boxy/cylindrical torso with natural equine musculature: prominent high withers ridge, sloping scapulae, deep chest & brisket keel, twin pectoral bulges, deep sprung barrel, and athletic abdominal flank tuck-up.
- **Arched Forward-Curving Neck & Sculpted Head**: Arched nuchal crest neck angled naturally forward; long tapered muzzle, defined jaw angles, nostrils, and mobile angled ears with pink inner ear cavities.
- **Anatomical Limb Articulation**: Fully calibrated 3-segment forelegs (scapula/humerus, forearm/radius, cannon bone, fetlock, pastern, flared hoof) and hindlegs (muscular thigh/femur, stifle, gaskin/tibia, hock with prominent calcaneus heel, cannon bone, hoof) achieving flush ground contact during standing and gaiting.
- **Gait Animation Continuity**: Retained smooth Walk, Trot, and Gallop transitions with authentic spine flex, body roll, and hoof impact particle triggers.

### 🏹 Medieval Kipchak Warrior Rider
- **Conical Iron Helmet**: Historical medieval Kipchak pointed iron helmet skull with bronze brow band and nasal arch.
- **Protective Armor & Aventail**: Hinged metal cheek guards and chainmail aventail draped over the warrior's neck and shoulders.
- **Stylized Dark Moustache**: Prominent, thick warrior moustache rigidly anchored to facial anatomy and head transforms.
- **Nomadic Riding Tunic & Gear**: Traditional nomadic tunic/kaftan, bronze belt buckle, boots in stirrups, and taut reins connected directly to the horse's bit.

---

## ✨ Features in Version 0.3

### 💨 Particle FX Engine
- **Hoof Dust Puffs**: Responsive dust particles spawned on ground impact during trotting and galloping.
- **Pickup Sparkles**: Golden burst feedback when snatching the serke.
- **Celebration Fireworks**: Traditional Kazakh turquoise, gold, ruby, and emerald celebration particles when scoring in the Kazanlyk.
- **Zero-Allocation Particle Pool**: High-performance pre-allocated particle system with drag, gravity, and ground bouncing.

### 🎯 Gameplay & Scoring Mechanics
- **Versatile Scoring**: Score either by riding directly into the Kazanlyk goal circle with the goat, or by flinging the goat from horseback into the goal circle!
- **Dynamic Throw Physics**: Throws inherit 125% horse velocity, outward launch arc, tumbling rotational inertia, and obstacle collision response.
- **Visual Feedback**: Pulsing ground pickup ring, bobbing beacon arrow, and celebratory gold score flash.

### 🧭 Polished Vector HUD & Minimap
- **Real-Time Circular Radar**: Circular radar minimap tracking arena boundary, Kazanlyk goal, goat position, and directional player arrow with golden carry glow.
- **Digital Speedometer**: Speed tier gauge with color transitions (Walk mint -> Trot gold -> Gallop ruby) and active gait indicator.
- **Digital Seven-Segment Score**: Retro-styled score display with score-flash celebrations.
- **Built-in Controls Cheat-Sheet**: On-screen reference panel toggleable with `H` or `F1`.

---

## 🛠️ Building & Running

### Requirements
- **OS**: Windows 10/11 (or Linux / macOS with OpenGL 3.3+)
- **Compiler**: MSVC 2022 (x64) or GCC / Clang with C++17 support
- **Build System**: CMake (>= 3.20) and Ninja

### Build Instructions (Windows / MSVC)
```powershell
# Open Developer PowerShell for VS 2022 or run vcvars64.bat
cmake --preset debug
cmake --build --preset debug

# Run the game
.\build\debug\kokpar.exe
```

