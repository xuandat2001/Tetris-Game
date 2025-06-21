# 🧩 Tetris Game on Nuvoton M487 Board (EEET2481 Assessment 3)

## 📘 Project Summary

This project is a complete embedded system implementation of the classic **Tetris** game developed for the **Nuvoton M487 Board** as part of **EEET2481 – Embedded Systems & Applications (Assessment 3)**.

All hardware control and game logic were implemented in **C language** using **Keil uVision5 IDE**. The game meets all functional and visual requirements outlined in the official assessment brief, including real-time control, custom graphics, scoring system, level progression, and state transitions.

---

## 🛠️ Development Tools

- **IDE**: [Keil uVision5](https://www.keil.com/download/product/)
- **Project Type**: Keil MDK ARM (µVision Project)
- **Microcontroller**: Nuvoton M487 Series (Cortex-M4)
- **Debugger**: Nu-Link (CMSIS-DAP)
- **Programming Language**: C
- **Display Interface**: EBI-connected LCD
- **Input Devices**: Joystick and Button SW1

---

## 🕹️ Game Overview

### 🎮 Game Layout

- **Grid Dimensions**: 24 (rows) × 32 (columns)
- **Each Unit Size**: 10 × 10 pixels
- **Play Area**: 14 units wide
- **Info Area**: 7 units wide
- **Boundary**: 1 unit on each side
- **Top Info Sections**:
  - `Next Block Preview`
  - `Status Panel`: High Score, Current Score, Time, Level
  - `Pause Info`: SW1 for pause

---

## 🧾 Game Features and Functionalities

| Feature                  | Description                                                                 |
|--------------------------|-----------------------------------------------------------------------------|
| **Welcome Stage**        | Custom welcome screen; shows on power-up or reset                           |
| **Play Stage**           | Gameplay starts after pressing SW1                                          |
| **Joystick Input**       | Left/Right: Move block; Up: Rotate 90° clockwise; Down: Drop instantly       |
| **SW1 Button**           | Pause/resume the game and control stage transitions                         |
| **Random Blocks**        | Each new block is randomly chosen from 7 Tetris blocks                      |
| **Line Clear**           | Full horizontal lines are detected and removed; blocks above fall down      |
| **Score System**         | +1 score for every line cleared                                             |
| **Leveling System**      | Level increases every 5 points; max level = 10                              |
| **Drop Speed**           | Faster block falling speed per level                                        |
| **Pause System**         | Timer, falling blocks, and inputs all pause on SW1 press                    |
| **Game Over**            | When block pile reaches the top row (row 31); switch to Game Over stage     |
| **High Score Stage**     | Displays top score from previous games; resets with new game                |
| **Played Time Display**  | Timer displayed in mm:ss format during gameplay                             |

---

## ⏱️ Drop Speed by Level

| Level | Speed (1 block per X seconds) |
|-------|-------------------------------|
| 1     | 0.50s                         |
| 2     | 0.45s                         |
| 3     | 0.40s                         |
| 4     | 0.35s                         |
| 5     | 0.30s                         |
| 6     | 0.25s                         |
| 7     | 0.20s                         |
| 8     | 0.15s                         |
| 9     | 0.10s                         |
| 10    | 0.05s                         |

Drop interval formula:
`Drop Speed = 1 unit / (0.5 - (Level - 1) * 0.05)`


---

## 🎨 Block Design

- Custom block shapes were designed and drawn on the LCD
- Seven Tetris blocks implemented:
  - **I**, **J**, **L**, **O**, **S**, **T**, **Z**
- Each block has a unique color and is 4 units in size
- Shapes drawn using `LCD_Draw_Rectangular()` function

---

## 🎯 Game States

The game is managed through a **finite state machine (FSM)**, with the following stages:

1. **WELCOME**
   - Displays welcome image/text
   - Press SW1 to continue

2. **READY TO PLAY**
   - Waits for SW1 to start the game

3. **PLAYING**
   - Blocks fall
   - Player interacts with joystick and button
   - Timer and score update in real time

4. **PAUSED**
   - Game paused by pressing SW1
   - Timer and blocks halted

5. **GAME OVER**
   - Triggered when blocks reach top boundary
   - Shows "Game Over" message

6. **HIGH SCORE**
   - Displays top scores in descending order
   - Waits for SW1 to start new game

---

## 🧱 Hardware Modules Used

| Module      | Purpose                              |
|-------------|---------------------------------------|
| **LCD (EBI)** | Display blocks, text, graphics        |
| **Timer 1**   | Controls block drop timing per level |
| **UART**      | Debug messages (optional)            |
| **Joystick**  | Movement and rotation                |
| **SW1 Button**| Game control and pause               |
| **GPIO**      | MFP and input configurations         |

---

## 📁 Project Folder Structure

├── EBI_LCD_Module.c / .h # Drawing utilities (blocks, rectangles, stars, etc.)
├── GameLoop.c / .h # The Loop of PLAING STATE
├── GameLogic.c / .h # Main FSM, movement, scoring, game over logic
├── main.c # Main program entry
├── Tetris_M487.uvprojx # Keil project file
├── README.md # Project documentation



---

## ▶️ How to Build and Run the Game

### 🧑‍💻 Requirements
- **Keil uVision5 IDE**
- **Nuvoton M487 Board**
- **USB Cable** for flashing/debugging

### 🧾 Steps

1. Open `Tetris_M487.uvprojx` in Keil uVision5
2. Select appropriate target device (e.g., M487JIDAE)
3. Connect the board via USB (Nu-Link)
4. Click **"Build"** or press `F7` to compile the project
5. Click **"Download"** or press `Ctrl + F8` to flash the program
6. Press **Reset** on the board to begin

---

## ✅ Requirements Checklist (✔️ = Implemented)

- [✔️] Custom game field design with unit boundary
- [✔️] 7-color Tetris blocks rendered
- [✔️] Joystick for move/rotate/drop
- [✔️] SW1 for Pause/Resume
- [✔️] Timed block drop with increasing speed
- [✔️] Line clear with score increment
- [✔️] Level up at every 5 lines cleared
- [✔️] High score tracking and display
- [✔️] Real-time timer (mm:ss)
- [✔️] Welcome, Play, Pause, Game Over, High Score stages
- [✔️] Fully demonstrated on M487 Board

---

## 🎥 Demonstration Tips (For Assessment)

- Start at **Welcome** screen
- Press **SW1** to enter **Play Stage**
- Show block movement with joystick (left/right)
- Rotate with joystick up
- Drop with joystick down
- Show scoring and level-up with line clears
- Pause/resume with SW1
- Show **Game Over** and **High Score** screens
- Re-enter game loop



