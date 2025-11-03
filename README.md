# STM32 Bare-Metal UART3 Driver
STM32 Bare-Metal UART3 Driver (Polling)
This project demonstrates how to implement a bare-metal UART driver for USART3 using STM32 microcontrollers. The example configures UART3 in polling mode for basic serial communication — sending and receiving data over PD8 (TX) and PD9 (RX) without any dependency on STM32Cube HAL or LL drivers.

🚀 Features
Bare-metal UART driver (no HAL or CMSIS overhead)

USART3 communication using polling mode

Transmit and receive single characters or strings

Example: Echo back received characters

🛠️ Setup Instructions
🔧 Prerequisites
STM32CubeIDE (latest version recommended)

STM32 Development Board (Tested on STM32H7; easily adaptable to STM32F4 or F7 series)

Serial Terminal (e.g., Tera Term, PuTTY) for UART testing

📁 Step 1: Create a New Project
Open STM32CubeIDE.

Create a new STM32 project with your target MCU (e.g., STM32H7A3ZI).

Choose the Empty Project option.

Finish the setup and let the IDE generate startup code.

📂 Step 2: Add Source Code
Replace your main.c with the provided UART3 driver code.

No HAL or CMSIS functions are required — it directly accesses registers via memory-mapped structs.

⚙️ Step 3: Build and Flash
Build the project using STM32CubeIDE.

Connect your board using ST-Link or J-Link.

Flash the firmware to the board.

Connect UART (TX on PD8, RX on PD9) to your PC using a USB-to-Serial converter.

Open a serial terminal at 115200 baud.

📄 Code Overview
Peripheral Setup
RCC enables clocks for GPIOD and USART3.

GPIOD pins 8 and 9 are set to alternate function mode AF7 for USART3.

USART3 is configured with:

115200 baud rate

Transmit/Receive enabled

Polling mode for simplicity

Functions
UART3_init() – Initializes GPIO and USART3.

UART3_send_char() – Sends one character.

UART3_send_string() – Sends a null-terminated string.

UART3_receive_char() – Receives one character (blocking).

main() – Sends a greeting and echoes received characters.
