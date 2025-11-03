#include <stdint.h>

// ------------------------------ Base Addresses ------------------------------
// Base memory addresses for RCC, GPIOD, and USART3 peripherals (specific to MCU)
#define RCC_BASE        (uint32_t)0x58024400
#define GPIOD_BASE      (uint32_t)0x58020C00
#define USART3_BASE     (uint32_t)0x40004800

// ------------------------------ Peripheral Structures ------------------------------
// RCC peripheral structure (only relevant registers for this example)
typedef struct
{
    uint32_t RESERVED[56];      /*!< Reserved memory area from 0x00 to 0xDC */
    uint32_t AHB4ENR;           /*!< AHB4 peripheral clock enable register at offset 0xE0 */
    uint32_t RESERVED1;         /*!< Reserved at offset 0xE4 */
    uint32_t APB1LENR;          /*!< APB1 Low peripheral clock enable register at offset 0xE8 */
} RCC_TypeDef;

// GPIO peripheral structure (only relevant registers used)
typedef struct
{
	 volatile uint32_t MODER;            /*!< GPIO port mode register at offset 0x00 */
	 volatile uint32_t RESERVED_1[7];    /*!< Reserved space to skip to AFR registers */
	 volatile uint32_t AFR[2];           /*!< Alternate function registers AFR[0] and AFR[1], offsets 0x20 and 0x24 */
} GPIO_TypeDef;

// USART peripheral structure (only relevant registers used)
typedef struct
{
	 volatile uint32_t CR1;               /*!< Control register 1, offset 0x00 */
	 volatile uint32_t RESERVED_1[2];    /*!< Reserved for CR2 and CR3, offsets 0x04 and 0x08 */
	 volatile uint32_t BRR;               /*!< Baud rate register, offset 0x0C */
	 volatile uint32_t RESERVED_2[3];    /*!< Reserved, offsets 0x10 to 0x18 */
	 volatile uint32_t ISR;               /*!< Interrupt and status register, offset 0x1C */
	 volatile uint32_t RESERVED_3;        /*!< Reserved at offset 0x20 */
	 volatile uint32_t RDR;               /*!< Receive data register, offset 0x24 */
	 volatile uint32_t TDR;               /*!< Transmit data register, offset 0x28 */
} USART_TypeDef;

// ------------------------------ Peripheral Pointers ------------------------------
// Create pointers to the peripherals using their base addresses and structs
#define RCC     ((RCC_TypeDef*)RCC_BASE)
#define GPIOD   ((GPIO_TypeDef*)GPIOD_BASE)
#define USART3  ((USART_TypeDef*)USART3_BASE)

// ------------------------------ UART Initialization ------------------------------
void UART3_init(void)
{
    // Enable clock for GPIOD (bit 3 in AHB4ENR) and USART3 (bit 18 in APB1LENR)
    RCC->AHB4ENR |= (1U << 3);    // Enable GPIOD clock
    RCC->APB1LENR |= (1U << 18);  // Enable USART3 clock

    // ----------------- Configure PD8 (TX) and PD9 (RX) pins -----------------

    // Set mode of PD8 and PD9 to Alternate Function (10 binary)
    GPIOD->MODER &= ~((3U << (8 * 2)) | (3U << (9 * 2)));  // Clear bits for PD8 and PD9 (each pin uses 2 bits)
    GPIOD->MODER |=  ((2U << (8 * 2)) | (2U << (9 * 2)));  // Set to Alternate Function mode (10)

    // Set alternate function to AF7 for PD8 and PD9 (USART3 functions)
    // PD8 and PD9 are in AFR[1] because pins 8-15 are controlled by AFR[1]
    GPIOD->AFR[1] &= ~((0xF << (0 * 4)) | (0xF << (1 * 4)));  // Clear AF bits for PD8 (pos 0) and PD9 (pos 1)
    GPIOD->AFR[1] |=  ((7U << (0 * 4)) | (7U << (1 * 4)));    // Set AF7 (USART3) for PD8 and PD9

    // ----------------- Configure USART3 -----------------

    // Set baud rate register for 115200 baud assuming APB1 clock = 64 MHz
    USART3->BRR = 64000000UL / 115200UL;

    // Enable receiver (RE) and transmitter (TE)
    USART3->CR1 |= (1U << 2);  // Set RE bit (bit 2) to enable receiver
    USART3->CR1 |= (1U << 3);  // Set TE bit (bit 3) to enable transmitter

    // Enable USART (UE bit)
    USART3->CR1 |= (1U << 0);  // Set UE bit (bit 0) to enable USART peripheral
}

// ------------------------------ UART Transmit ------------------------------
// Send a single character via USART3
void UART3_send_char(char c) {
    while (!(USART3->ISR & (1U << 7)));  // Wait until transmit data register empty (TXE=1)
    USART3->TDR = c;                      // Write character to transmit data register
}

// Send a null-terminated string via USART3
void UART3_send_string(const char *str) {
    while (*str) {
        UART3_send_char(*str++);          // Send each character until null terminator
    }
}

// ------------------------------ UART Receive (Polling) ------------------------------
// Receive a single character via USART3 (polling mode)
char UART3_receive_char(void) {
    while (!(USART3->ISR & (1U << 5)));  // Wait until receive data register not empty (RXNE=1)
    return USART3->RDR;                   // Read received character from data register
}

// ------------------------------ Main ------------------------------
int main(void)
{
    UART3_init();                        // Initialize UART3 peripheral

    UART3_send_string("Hello IEE4!\r\n");  // Send initial greeting string

    while (1) {
        char c = UART3_receive_char();  // Receive character from UART (blocking)
        UART3_send_char(c);              // Echo back received character
    }
}
