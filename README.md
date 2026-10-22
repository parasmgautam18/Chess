# ♟️ Bitboard Chess — C++ Chess Game with an AI Engine

A fully playable desktop chess game written in **C++** using **bitboards** and the **[raylib](https://www.raylib.com/)** graphics library. Play as White or Black against a built-in chess engine that plays at roughly the level of **Stockfish limited to ~1700 Elo** (an informal estimate, not an official rating).

The game follows the rules of chess, including castling, en passant, pawn promotion, check, checkmate and stalemate.

---

## Table of Contents

- [Features](#features)
- [How the Engine Works](#how-the-engine-works)
- [Requirements](#requirements)
- [Installation and Running](#installation-and-running)
  - [Windows](#windows)
  - [macOS](#macos)
  - [Linux](#linux)
- [How to Play](#how-to-play)
- [Configuration](#configuration)
- [Project Structure](#project-structure)
- [Credits](#credits)

---

## Features

- Play as **White** or **Black** (the board flips automatically when you play Black)
- Complete chess rules: **castling**, **en passant**, **pawn promotion** (queen, rook, bishop or knight), **check**, **checkmate** and **stalemate**
- Click-to-move interface with **legal-move indicators**, **last-move highlighting** and a **red highlight on a king in check**
- AI opponent with roughly **~1700 Elo** strength
- Clean 800×800 window with image-based pieces

## How the Engine Works

| Component | Implementation |
|---|---|
| Board representation | 12 × 64-bit **bitboards** (one per piece type and colour) |
| Move generation | Fully legal moves, including castling (with attacked-square checks), en passant and promotions |
| Search | **Negamax** with **alpha-beta pruning**, 5 plies by default |
| Quiescence search | Continues searching captures and promotions past the depth limit to avoid tactical blunders |
| Move ordering | **MVV-LVA** (Most Valuable Victim – Least Valuable Attacker) plus promotion bonuses |
| Evaluation | Material values, **piece-square tables**, and a bishop-pair bonus |
| Mate handling | Mate scores include the ply distance, so the engine prefers faster mates |

---

## Requirements

| Requirement | Details |
|---|---|
| **C++ compiler** | **GCC (g++)** with C++17 support. The code uses `<bits/stdc++.h>` and GCC/Clang built-ins (`__builtin_ctzll`, `__builtin_popcountll`), so **MSVC (Visual Studio) is not supported** |
| **raylib** | Version **5.5 or newer** |
| **Git** | To clone the repository |
| **Graphics** | A GPU/driver with **OpenGL 3.3** support |
| **Assets** | The `assets/` folder with the 12 piece images (already included in the repository) |

> **Important:** The game loads piece images from the relative path `assets/`. Always run the game **from the root folder of the repository**, otherwise the pieces will not appear.

---

## Installation and Running

Choose your operating system. Each section is complete: install the tools, clone the repo, build the game and run it.

In every command below, replace `YOUR_USERNAME/YOUR_REPO` with the real GitHub path of this repository, and `main.cpp` with the actual name of the source file if it differs.

---

### Windows

On Windows the easiest way to get GCC and raylib is **MSYS2**.

**1. Install Git and MSYS2**

- Git: download from [git-scm.com](https://git-scm.com/download/win)
- MSYS2: download from [msys2.org](https://www.msys2.org/) and install it with the default options

Or install both from a terminal with `winget`:

```powershell
winget install --id Git.Git -e
winget install --id MSYS2.MSYS2 -e
```

**2. Open the correct terminal**

From the Start menu, open **"MSYS2 UCRT64"**. Do not use "MSYS2 MSYS" or "MSYS2 MINGW64" for the commands below.

**3. Update MSYS2 and install the compiler, raylib and Git**

```bash
pacman -Syu
```

If it asks you to close the terminal, close it, reopen **MSYS2 UCRT64** and run `pacman -Syu` once more. Then install the tools:

```bash
pacman -S --needed git mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib
```

**4. Clone the repository**

```bash
git clone https://github.com/YOUR_USERNAME/YOUR_REPO.git
cd YOUR_REPO
```

To put it in a normal Windows folder instead, first run, for example, `cd /c/Users/YourName/Documents`.

**5. Build the game**

```bash
g++ chess.cpp -o chess.exe -O3 -std=c++17 -lraylib -lopengl32 -lgdi32 -lwinmm
```

**6. Run the game**

```bash
./chess.exe
```

> **Tip:** If you want a standalone `chess.exe` that also runs when double-clicked outside the MSYS2 terminal (without needing `raylib.dll`), add `-static` to the build command:
>
> ```bash
> g++ chess.cpp -o chess.exe -O3 -std=c++17 -static -lraylib -lopengl32 -lgdi32 -lwinmm
> ```
>
> Keep the `assets/` folder next to the `.exe`.

**Alternative: WSL2.** On Windows 11 (or recent Windows 10), you can install Ubuntu through WSL2 and follow the [Linux (Ubuntu / Debian)](#ubuntu--debian--linux-mint) steps. GUI apps run through WSLg.

---

### macOS

The code uses `<bits/stdc++.h>`, which Apple's default compiler does not include. Pick **one** of the two options below. Option A needs no code changes.

**1. Install the command line tools and Homebrew**

```bash
xcode-select --install
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

On Apple Silicon Macs, run the "Next steps" commands that the Homebrew installer prints so that `brew` is on your `PATH`.

**2. Install Git, GCC and raylib**

```bash
brew install git gcc raylib
```

**3. Clone the repository**

```bash
git clone https://github.com/YOUR_USERNAME/YOUR_REPO.git
cd YOUR_REPO
```

**4. Build the game**

#### Option A — Homebrew GCC (no code changes)

Homebrew installs GCC under a versioned name such as `g++-15`. This command finds the newest one automatically:

```bash
GXX=$(ls "$(brew --prefix)"/bin/g++-* | sort -V | tail -n 1)
RAYLIB=$(brew --prefix raylib)

"$GXX" chess.cpp -o chess -O3 -std=c++17 -I"$RAYLIB/include" -L"$RAYLIB/lib" -lraylib
```

#### Option B — Apple Clang (one small edit)

At the top of `main.cpp`, replace

```cpp
#include <bits/stdc++.h>
```

with

```cpp
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>
```

Then build:

```bash
RAYLIB=$(brew --prefix raylib)

clang++ chess.cpp -o chess -O3 -std=c++17 -I"$RAYLIB/include" -L"$RAYLIB/lib" -lraylib
```

**5. Run the game**

```bash
./chess
```

---

### Linux

Pick the section for your distribution. In all cases, run the game from the repository root.

#### Ubuntu / Debian / Linux Mint

**1. Install the compiler, Git, CMake and raylib's build dependencies**

```bash
sudo apt update
sudo apt install -y build-essential git cmake \
    libasound2-dev libx11-dev libxrandr-dev libxi-dev libxcursor-dev \
    libxinerama-dev libgl1-mesa-dev libglu1-mesa-dev \
    libwayland-dev libxkbcommon-dev
```

**2. Build and install raylib**

```bash
git clone --depth 1 --branch 5.5 https://github.com/raysan5/raylib.git
cd raylib
cmake -B build -DBUILD_EXAMPLES=OFF
cmake --build build -j"$(nproc)"
sudo cmake --install build
sudo ldconfig
cd ..
```

**3. Clone this repository**

```bash
git clone https://github.com/YOUR_USERNAME/YOUR_REPO.git
cd YOUR_REPO
```

**4. Build the game**

```bash
g++ chess.cpp -o chess -O3 -std=c++17 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

**5. Run the game**

```bash
./chess
```

#### Fedora

```bash
sudo dnf install -y gcc-c++ git raylib-devel mesa-libGL-devel libX11-devel

git clone https://github.com/YOUR_USERNAME/YOUR_REPO.git
cd YOUR_REPO

g++ chess.cpp -o chess -O3 -std=c++17 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
./chess
```

If your Fedora release does not provide `raylib-devel`, build raylib from source using the Ubuntu / Debian steps above (install the equivalent `-devel` packages instead of the `apt` ones).

#### Arch Linux / Manjaro

```bash
sudo pacman -S --needed base-devel git raylib

git clone https://github.com/YOUR_USERNAME/YOUR_REPO.git
cd YOUR_REPO

g++ chess.cpp -o chess -O3 -std=c++17 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
./chess
```

---

### Build Command Cheat Sheet

| OS | Build command | Run command |
|---|---|---|
| **Windows** (MSYS2 UCRT64) | `g++ chess.cpp -o chess.exe -O3 -std=c++17 -lraylib -lopengl32 -lgdi32 -lwinmm` | `./chess.exe` |
| **macOS** (Homebrew GCC) | `"$GXX" chess.cpp -o chess -O3 -std=c++17 -I"$RAYLIB/include" -L"$RAYLIB/lib" -lraylib` | `./chess` |
| **Linux** | `g++ chess.cpp -o chess -O3 -std=c++17 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11` | `./chess` |

> Always keep **`-O3`**. The engine searches 5 plies, and an unoptimized build can be many times slower.

---

## How to Play

1. Start the game. A menu asks you to **choose your side**: click **WHITE** or **BLACK**.
2. **Click one of your pieces** to select it. Legal destinations are shown as dots (empty squares) or rings (captures).
3. **Click a highlighted square** to move. Click another of your own pieces to change your selection.
4. The engine replies after a short delay (about 2 seconds).
5. Close the window or press **Esc** to quit.

| Action | How |
|---|---|
| Move a piece | Click the piece, then click a highlighted square |
| Castle | Select your king and click the square two files over (g1/c1 for White, g8/c8 for Black) |
| En passant | Select the pawn and click the highlighted en passant square |
| Promote a pawn | Move it to the last rank, then click the piece you want in the popup |
| Return to the menu | Press **R** once the game is over |

The board highlights the last move, and a king in check is shown in red. The game ends automatically on **checkmate** or **stalemate**.

---

## Configuration

These constants are near the top of the source file. After changing any of them, **rebuild** the game.

| Constant | Default | Meaning |
|---|---|---|
| `SEARCH_DEPTH` | `5` | Engine search depth in plies. Lower (e.g. `4`) is faster and weaker; higher (e.g. `6`) is stronger and noticeably slower |
| `BOT_DELAY` | `2.0` | Seconds the engine waits before moving (inside `main`) |
| `TILE_SIZE` | `100` | Size of one square in pixels. The menu layout assumes the 800×800 window, so changing this is not recommended |

Board colours can be changed in `main` (`lightSquare`, `darkSquare`, `highlightColor`, `lastMoveColor`, `checkColor`).

---

## Project Structure

```text
YOUR_REPO/
├── main.cpp          # Game logic, move generation, AI engine and raylib UI
├── assets/           # Piece images
│   ├── wP.png  wN.png  wB.png  wR.png  wQ.png  wK.png
│   └── bP.png  bN.png  bB.png  bR.png  bQ.png  bK.png
└── README.md
```
---

## Credits

- [raylib](https://www.raylib.com/) by Ramon Santamaria (@raysan5), used for the window, input and graphics
- The piece-square tables are based on the *Simplified Evaluation Function* from the [Chess Programming Wiki](https://www.chessprogramming.org/Simplified_Evaluation_Function)