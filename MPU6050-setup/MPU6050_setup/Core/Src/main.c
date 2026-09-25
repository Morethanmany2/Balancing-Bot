#include "stm32f1xx.h"


/* =========================================================
 * FUNCTION PROTOTYPES
 * =========================================================
 */

/* Clock */
void Clock_Init(void);

/* GPIO */
void GPIO_Init(void);

/* UART */
void UART1_Init(void);
void UART1_SendChar(char c);
void UART1_SendString(char *str);
void UART1_SendHex(uint8_t value);

/* I2C */
void I2C1_Init(void);
void I2C1_Start(void);
void I2C1_Stop(void);
void I2C1_Write(uint8_t data);
uint8_t I2C1_Read_ACK(void);
uint8_t I2C1_Read_NACK(void);

/* MPU6050 */
#define MPU6050_ADDR       0x68
#define MPU6050_WHO_AM_I   0x75

uint8_t MPU6050_ReadRegister(uint8_t reg);


/* =========================================================
 * MAIN
 * =========================================================
 */

int main(void)
{
    /*
     * Step 1:
     * Configure system clock.
     *
     * 8 MHz HSE
     *     ↓
     * PLL ×9
     *     ↓
     * 72 MHz SYSCLK
     */
    Clock_Init();


    /*
     * Step 2:
     * Configure GPIO ports.
     *
     * PA9  → USART1 TX
     * PA10 → USART1 RX
     *
     * PB6  → I2C1 SCL
     * PB7  → I2C1 SDA
     */
    GPIO_Init();


    /*
     * Step 3:
     * Configure USART1.
     *
     * Baud rate = 115200
     */
    UART1_Init();


    /*
     * Step 4:
     * Configure I2C1.
     *
     * I2C clock = 100 kHz
     */
    I2C1_Init();


    /*
     * Small startup message.
     */
    UART1_SendString("STM32 Started\r\n");


    /*
     * Read MPU6050 WHO_AM_I register.
     *
     * Register = 0x75
     *
     * Expected value:
     * 0x68
     */
    uint8_t who_am_i;

    who_am_i = MPU6050_ReadRegister(MPU6050_WHO_AM_I);


    /*
     * Print result through UART.
     */
    UART1_SendString("WHO_AM_I = 0x");

    UART1_SendHex(who_am_i);

    UART1_SendString("\r\n");


    /*
     * Main loop
     */
    while (1)
    {

    }
}


/* =========================================================
 * CLOCK INITIALIZATION
 * =========================================================
 *
 * HSE  = 8 MHz
 * PLL  = ×9
 * SYSCLK = 72 MHz
 *
 * APB1 = 36 MHz
 * APB2 = 72 MHz
 * =========================================================
 */

void Clock_Init(void)
{
    /* -----------------------------------------------------
     * Enable HSE
     *
     * RCC->CR bit 16 = HSEON
     * -----------------------------------------------------
     */

    RCC->CR |= (1 << 16);


    /* -----------------------------------------------------
     * Wait until HSE is ready
     *
     * RCC->CR bit 17 = HSERDY
     * -----------------------------------------------------
     */

    while (!(RCC->CR & (1 << 17)))
    {
    }


    /* -----------------------------------------------------
     * Flash configuration
     *
     * Bit 4 = Prefetch enable
     * -----------------------------------------------------
     */

    FLASH->ACR |= (1 << 4);


    /*
     * Bits [2:0] = Flash latency
     *
     * Clear them first.
     */

    FLASH->ACR &= ~(0x7 << 0);


    /*
     * Set latency = 2 wait states.
     */

    FLASH->ACR |= (0x2 << 0);


    /* -----------------------------------------------------
     * PLL source = HSE
     *
     * RCC->CFGR bit 16 = PLLSRC
     *
     * 1 → HSE
     * -----------------------------------------------------
     */

    RCC->CFGR |= (1 << 16);


    /* -----------------------------------------------------
     * PLL multiplier = ×9
     *
     * RCC->CFGR bits [21:18]
     *
     * ×9 = 0111
     * -----------------------------------------------------
     */

    RCC->CFGR &= ~(0xF << 18);

    RCC->CFGR |= (0x7 << 18);


    /* -----------------------------------------------------
     * APB1 prescaler = /2
     *
     * RCC->CFGR bits [10:8]
     *
     * 100 → /2
     *
     * 72 MHz / 2 = 36 MHz
     * -----------------------------------------------------
     */

    RCC->CFGR &= ~(0x7 << 8);

    RCC->CFGR |= (0x4 << 8);


    /* -----------------------------------------------------
     * Enable PLL
     *
     * RCC->CR bit 24 = PLLON
     * -----------------------------------------------------
     */

    RCC->CR |= (1 << 24);


    /* -----------------------------------------------------
     * Wait until PLL ready
     *
     * RCC->CR bit 25 = PLLRDY
     * -----------------------------------------------------
     */

    while (!(RCC->CR & (1 << 25)))
    {
    }


    /* -----------------------------------------------------
     * Select PLL as SYSCLK
     *
     * RCC->CFGR bits [1:0]
     *
     * 10 → PLL
     * -----------------------------------------------------
     */

    RCC->CFGR &= ~(0x3 << 0);

    RCC->CFGR |= (0x2 << 0);


    /* -----------------------------------------------------
     * Wait until PLL is actually being used as SYSCLK
     *
     * RCC->CFGR bits [3:2] = SWS
     *
     * 10 → PLL
     * -----------------------------------------------------
     */

    while ((RCC->CFGR & (0x3 << 2)) != (0x2 << 2))
    {
    }
}


/* =========================================================
 * GPIO INITIALIZATION
 * =========================================================
 *
 * GPIOA:
 *
 * PA9  → USART1 TX
 * PA10 → USART1 RX
 *
 *
 * GPIOB:
 *
 * PB6 → I2C1 SCL
 * PB7 → I2C1 SDA
 * =========================================================
 */

void GPIO_Init(void)
{
    /* -----------------------------------------------------
     * Enable GPIOA clock
     *
     * RCC->APB2ENR
     *
     * Bit 2 = IOPAEN
     * -----------------------------------------------------
     */

    RCC->APB2ENR |= (1 << 2);


    /* -----------------------------------------------------
     * Enable GPIOB clock
     *
     * Bit 3 = IOPBEN
     * -----------------------------------------------------
     */

    RCC->APB2ENR |= (1 << 3);


    /* -----------------------------------------------------
     * Enable AFIO clock
     *
     * Bit 0 = AFIOEN
     *
     * Needed for alternate-function pins.
     * -----------------------------------------------------
     */

    RCC->APB2ENR |= (1 << 0);


    /* =====================================================
     * PA9 → USART1 TX
     * =====================================================
     *
     * PA9 is in GPIOA->CRH
     *
     * PA8  → bits 3:0
     * PA9  → bits 7:4
     *
     * USART TX:
     *
     * MODE = 11 → Output 50 MHz
     * CNF  = 10 → Alternate Function Push-Pull
     *
     * Binary:
     *
     * CNF MODE
     *  10   11
     * 1011
     *
     * Therefore PA9 = 1011
     */

    GPIOA->CRH &= ~(0xF << 4);

    GPIOA->CRH |= (0xB << 4);


    /* =====================================================
     * PA10 → USART1 RX
     * =====================================================
     *
     * RX is input.
     *
     * MODE = 00
     * CNF  = 01 → Floating input
     *
     * 0001
     */

    GPIOA->CRH &= ~(0xF << 8);

    GPIOA->CRH |= (0x1 << 8);


    /* =====================================================
     * PB6 → I2C1 SCL
     * =====================================================
     *
     * I2C requires:
     *
     * Alternate Function
     * Open Drain
     *
     * MODE = 11 → Output 50 MHz
     * CNF  = 11 → Alternate Function Open Drain
     *
     * 1111
     *
     * PB6 is in CRL:
     *
     * PB0 → bits 3:0
     * PB6 → bits 27:24
     */

    GPIOB->CRL &= ~(0xF << 24);

    GPIOB->CRL |= (0xF << 24);


    /* =====================================================
     * PB7 → I2C1 SDA
     * =====================================================
     *
     * Same configuration:
     *
     * MODE = 11
     * CNF  = 11
     *
     * 1111
     *
     * PB7 → bits [31:28]
     */

    GPIOB->CRL &= ~(0xF << 28);

    GPIOB->CRL |= (0xF << 28);
}


/* =========================================================
 * USART1 INITIALIZATION
 * =========================================================
 *
 * USART1 clock:
 *
 * APB2 = 72 MHz
 *
 * Desired baud rate:
 *
 * 115200
 *
 * USARTDIV = 72,000,000 / (16 × 115200)
 *          ≈ 39.0625
 *
 * BRR = 0x0271 approximately
 * =========================================================
 */

void UART1_Init(void)
{
    /* -----------------------------------------------------
     * Enable USART1 clock
     *
     * RCC->APB2ENR
     *
     * Bit 14 = USART1EN
     * -----------------------------------------------------
     */

    RCC->APB2ENR |= (1 << 14);


    /* -----------------------------------------------------
     * USART configuration
     *
     * CR1:
     *
     * Bit 13 = UE  → USART enable
     * Bit 3  = TE  → Transmitter enable
     * Bit 2  = RE  → Receiver enable
     *
     * 8-bit word length
     * No parity
     * -----------------------------------------------------
     */

    USART1->CR1 = 0;


    /* -----------------------------------------------------
     * Baud rate = 115200
     *
     * APB2 clock = 72 MHz
     *
     * BRR = 0x0271
     * -----------------------------------------------------
     */

    USART1->BRR = 0x0271;


    /* -----------------------------------------------------
     * Enable transmitter
     *
     * CR1 bit 3 = TE
     * -----------------------------------------------------
     */

    USART1->CR1 |= (1 << 3);


    /* -----------------------------------------------------
     * Enable receiver
     *
     * CR1 bit 2 = RE
     * -----------------------------------------------------
     */

    USART1->CR1 |= (1 << 2);


    /* -----------------------------------------------------
     * Enable USART
     *
     * CR1 bit 13 = UE
     * -----------------------------------------------------
     */

    USART1->CR1 |= (1 << 13);
}


/* =========================================================
 * UART SEND CHARACTER
 * =========================================================
 */

void UART1_SendChar(char c)
{
    /*
     * USART1->SR = Status Register
     *
     * Bit 7 = TXE
     *
     * TXE = 1 means:
     *
     * Transmit Data Register is empty.
     */

    while (!(USART1->SR & (1 << 7)))
    {
        // Wait until transmit register is empty
    }


    /*
     * USART1->DR = Data Register
     *
     * Writing data here starts transmission.
     */

    USART1->DR = c;
}


/* =========================================================
 * UART SEND STRING
 * =========================================================
 */

void UART1_SendString(char *str)
{
    /*
     * Keep sending characters until
     * null terminator '\0' is reached.
     */

    while (*str)
    {
        UART1_SendChar(*str);

        str++;
    }
}


/* =========================================================
 * UART SEND HEX BYTE
 * =========================================================
 *
 * Example:
 *
 * value = 0x68
 *
 * Output:
 *
 * 68
 * =========================================================
 */

void UART1_SendHex(uint8_t value)
{
    char hex[] = "0123456789ABCDEF";


    /*
     * Upper nibble:
     *
     * 0x68
     *
     * 0x68 >> 4 = 0x06
     */

    UART1_SendChar(hex[(value >> 4) & 0x0F]);


    /*
     * Lower nibble:
     *
     * 0x68 & 0x0F = 0x08
     */

    UART1_SendChar(hex[value & 0x0F]);
}


/* =========================================================
 * I2C1 INITIALIZATION
 * =========================================================
 *
 * APB1 clock = 36 MHz
 *
 * I2C speed = 100 kHz
 *
 * Standard mode.
 * =========================================================
 */

void I2C1_Init(void)
{
    /* -----------------------------------------------------
     * Enable I2C1 peripheral clock
     *
     * RCC->APB1ENR
     *
     * Bit 21 = I2C1EN
     * -----------------------------------------------------
     */

    RCC->APB1ENR |= (1 << 21);


    /* -----------------------------------------------------
     * Reset I2C peripheral
     *
     * CR1 bit 15 = SWRST
     *
     * 1 → reset
     * 0 → normal operation
     * -----------------------------------------------------
     */

    I2C1->CR1 |= (1 << 15);

    I2C1->CR1 &= ~(1 << 15);


    /* -----------------------------------------------------
     * Configure peripheral input clock frequency
     *
     * I2C1 is connected to APB1.
     *
     * APB1 = 36 MHz
     *
     * CR2 bits [5:0] = FREQ
     *
     * FREQ = 36
     * -----------------------------------------------------
     */

    I2C1->CR2 &= ~(0x3F);

    I2C1->CR2 |= 36;


    /* -----------------------------------------------------
     * Configure clock control register
     *
     * CCR bits [11:0]
     *
     * Standard mode:
     *
     * CCR = APB1 clock / (2 × I2C clock)
     *
     * CCR = 36,000,000 / (2 × 100,000)
     *
     * CCR = 180
     * -----------------------------------------------------
     */

    I2C1->CCR &= ~(0xFFF);

    I2C1->CCR |= 180;


    /* -----------------------------------------------------
     * Maximum rise time
     *
     * TRISE = FREQ + 1
     *
     * FREQ = 36 MHz
     *
     * Therefore:
     *
     * TRISE = 37
     * -----------------------------------------------------
     */

    I2C1->TRISE = 37;


    /* -----------------------------------------------------
     * Enable I2C peripheral
     *
     * CR1 bit 0 = PE
     * -----------------------------------------------------
     */

    I2C1->CR1 |= (1 << 0);
}


/* =========================================================
 * I2C START CONDITION
 * =========================================================
 */

void I2C1_Start(void)
{
    /*
     * CR1 bit 8 = START
     *
     * Writing 1 generates START condition.
     */

    I2C1->CR1 |= (1 << 8);


    /*
     * Wait for SB flag.
     *
     * SR1 bit 0 = SB
     *
     * SB = 1 means START condition generated.
     */

    while (!(I2C1->SR1 & (1 << 0)))
    {
    }
}


/* =========================================================
 * I2C STOP CONDITION
 * =========================================================
 */

void I2C1_Stop(void)
{
    /*
     * CR1 bit 9 = STOP
     *
     * Writing 1 generates STOP condition.
     */

    I2C1->CR1 |= (1 << 9);
}


/* =========================================================
 * I2C WRITE ONE BYTE
 * =========================================================
 */

void I2C1_Write(uint8_t data)
{
    /*
     * DR = Data Register
     *
     * Writing here sends one byte.
     */

    I2C1->DR = data;


    /*
     * Wait for TXE.
     *
     * SR1 bit 7 = TXE
     *
     * TXE = 1 means DR is empty.
     */

    while (!(I2C1->SR1 & (1 << 7)))
    {
    }
}


/* =========================================================
 * I2C READ WITH ACK
 * =========================================================
 */

uint8_t I2C1_Read_ACK(void)
{
    uint8_t data;


    /*
     * CR1 bit 10 = ACK
     *
     * Enable ACK before receiving.
     */

    I2C1->CR1 |= (1 << 10);


    /*
     * Wait until RXNE.
     *
     * SR1 bit 6 = RXNE
     *
     * RXNE = 1 means received data available.
     */

    while (!(I2C1->SR1 & (1 << 6)))
    {
    }


    /*
     * Read received byte.
     */

    data = I2C1->DR;


    return data;
}


/* =========================================================
 * I2C READ WITH NACK
 * =========================================================
 */

uint8_t I2C1_Read_NACK(void)
{
    uint8_t data;


    /*
     * Disable ACK.
     *
     * This causes NACK after receiving
     * the final byte.
     */

    I2C1->CR1 &= ~(1 << 10);


    /*
     * Wait for received data.
     */

    while (!(I2C1->SR1 & (1 << 6)))
    {
    }


    /*
     * Read data.
     */

    data = I2C1->DR;


    return data;
}


/* =========================================================
 * MPU6050 READ REGISTER
 * =========================================================
 *
 * Sequence:
 *
 * START
 * ↓
 * MPU6050 address + WRITE
 * ↓
 * Register address
 * ↓
 * REPEATED START
 * ↓
 * MPU6050 address + READ
 * ↓
 * Receive data
 * ↓
 * NACK
 * ↓
 * STOP
 * =========================================================
 */

uint8_t MPU6050_ReadRegister(uint8_t reg)
{
    uint8_t data;


    /* -----------------------------------------------------
     * START
     * -----------------------------------------------------
     */

    I2C1_Start();


    /* -----------------------------------------------------
     * Send MPU6050 address + WRITE
     *
     * MPU6050 address = 0x68
     *
     * I2C address is 7-bit.
     *
     * Shift left by 1:
     *
     * 0x68 << 1 = 0xD0
     *
     * Write bit = 0
     *
     * Therefore:
     *
     * 0xD0
     * -----------------------------------------------------
     */

    I2C1_Write(MPU6050_ADDR << 1);


    /*
     * Wait for ACK from MPU6050.
     *
     * SR1 bit 1 = ADDR
     *
     * ADDR = 1 means address acknowledged.
     */

    while (!(I2C1->SR1 & (1 << 1)))
    {
    }


    /*
     * Clear ADDR flag.
     *
     * According to STM32F1 I2C hardware,
     * ADDR is cleared by reading SR1 followed by SR2.
     */

    volatile uint32_t temp;

    temp = I2C1->SR1;

    temp = I2C1->SR2;

    (void)temp;


    /* -----------------------------------------------------
     * Send MPU6050 register address
     *
     * Example:
     *
     * WHO_AM_I = 0x75
     * -----------------------------------------------------
     */

    I2C1_Write(reg);


    /* -----------------------------------------------------
     * Wait until byte transmission finished.
     *
     * BTF = Bit Transfer Finished
     *
     * SR1 bit 2 = BTF
     * -----------------------------------------------------
     */

    while (!(I2C1->SR1 & (1 << 2)))
    {
    }


    /* -----------------------------------------------------
     * REPEATED START
     * -----------------------------------------------------
     */

    I2C1_Start();


    /* -----------------------------------------------------
     * Send MPU6050 address + READ
     *
     * 0x68 << 1 = 0xD0
     *
     * Set bit 0:
     *
     * 0xD1
     * -----------------------------------------------------
     */

    I2C1_Write((MPU6050_ADDR << 1) | 1);


    /*
     * Wait for address acknowledgement.
     */

    while (!(I2C1->SR1 & (1 << 1)))
    {
    }


    /*
     * Clear ADDR.
     */

    temp = I2C1->SR1;

    temp = I2C1->SR2;

    (void)temp;


    /* -----------------------------------------------------
     * Read final byte.
     *
     * We want only ONE byte.
     *
     * Therefore use NACK.
     * -----------------------------------------------------
     */

    data = I2C1_Read_NACK();


    /* -----------------------------------------------------
     * STOP
     * -----------------------------------------------------
     */

    I2C1_Stop();


    return data;
}
