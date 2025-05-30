# 💳 ATM Banking System – Full Stack Simulation

This project simulates a complete ATM-Banking architecture, integrating embedded firmware, Linux-based ATM backend code, and a central banking application with database interaction. It's modular, scalable, and meant for both simulation and real-device testing (e.g., LPC2148).

## 📦 Project Structure

    majorz/
    ├── atmz/         # ATM backend logic written in C (Linux-based)
    ├── firmwarez/    # Embedded firmware for LPC2148 (front-end interface)
    ├── bankz/        # Central banking application (admin control, info, and DB)
    ├── dataz/        # Database storage (data files)
    ├── filez/        # Human-readable transaction sheets and logs
    ├── atm_client/   # (WIP) ATM client simulation for future ATM instances
    ├── atm_server/   # (WIP) Concurrent server to handle ATM requests

## 🧠 Project Explanation

### ✅ atmz/ – ATM Backend

This component simulates the backend logic of an ATM, including:

- User authentication via PIN  
- Balance check  
- Withdrawal and deposit simulation  
- Interaction with stored data files  
- Designed to run in a Linux environment using the `make` build system  

Run this to simulate ATM operation from a terminal.

### ✅ firmwarez/ – LPC2148 Frontend

- Embedded C code to display ATM interface on real hardware (LPC2148)
- Handles:
  - Keypad inputs (PIN, amount)
  - LCD display outputs (welcome, processing, result)
  - UART communication with ATM backend
- Can be flashed using Keil or Flash Magic

### ✅ bankz/ – Central Bank Application

- Admin-side application to control and monitor:
  - Account info (create/update/delete)
  - Transaction logs
  - Balance database
- Also simulates backend DB interactions
- Intended to run in Linux using `make`

### ⚙️ dataz/ – Database

- Contains persistent files storing:
  - Account info
  - Transaction history
- These files are read and written by both `atmz` and `bankz`

### 📄 filez/ – Logs and Sheets

- CSV or TXT format records for:
  - Human-readable transaction logs
  - Admin audit reports

### 🧪 atm_client/ & atm_server/ – Under Development

- Intended for future implementation of:
  - Multi-ATM simulations using TCP/IP sockets
  - A concurrent server that handles requests from multiple ATM clients via threads

> These components are currently under development. Use `atmz`, `bankz`, and `firmwarez` for the working demo.

## 🚀 How to Run (Beginner Friendly)

### 1. Download the ZIP

Click **Code > Download ZIP** and unzip.

Or use terminal:

    wget https://github.com/<your-username>/<your-repo>/archive/refs/heads/main.zip
    unzip main.zip
    cd <your-repo>-main/majorz

### 2. Install Build Tools

Make sure you have `make` and `gcc`.

Ubuntu/Debian:

    sudo apt update
    sudo apt install build-essential

Arch:

    sudo pacman -S base-devel

### 3. Compile and Run ATM Backend

    cd atmz
    make
    ./atm

### 4. Compile and Run Bank Application

    cd ../bankz
    make
    ./bankz

### 5. (Optional) Load Firmware to LPC2148

If using real hardware:

- Open Keil or Flash Magic  
- Load hex/bin files from `firmwarez/`  
- Flash to LPC2148  
- Connect serial to ATM backend  

## 📌 Notes

- This project can be scaled to simulate a real ATM network with multiple clients and a central server.
- Data files are simple, human-readable formats for ease of debugging and visualization.

## 🧑‍💻 Contributing

Pull requests are welcome. Please fork the repository and create a branch for improvements.

## 📝 License

Open-source for academic and learning purposes.
