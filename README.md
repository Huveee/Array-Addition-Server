
# Array Addition Server

## Overview
This project implements an Array Addition Server as part of the Operating Systems course assignment at Dokuz Eylul University. The server operates on Linux and is designed to handle array addition requests via telnet connection. It supports handling two integer arrays, adding corresponding elements, and managing potential carries when sums exceed three digits.

## Features
- Receives two integer arrays via telnet.
- Performs element-wise addition of the arrays.
- Handles integer overflows by carrying over excess amounts.
- Validates input to ensure that all elements are integers and both arrays have the same number of elements.
- Supports error handling for various input issues.

## Requirements
- Linux Operating System
- GCC for compiling the C program
- Telnet for client-server communication
- VirtualBox for testing in a virtual environment

## Installation
1. Clone the repository:
   ```bash
   git clone <repository-url>
   ```
2. Compile the program using GCC:
   ```bash
   gcc -o array_add_server array_add_server.c
   ```

## Usage
Start the server by running:
```bash
./array_add_server
```
Connect to the server using telnet:
```bash
telnet localhost 60000
```
Follow the prompts to input the integer arrays.

## Acknowledgments
- Course: CME 3205 Operating Systems, Fall Semester 2023/2024
