#ifndef CONFIG_H
#define CONFIG_H

// We assume that scan lines are 4 consecutive lines in one port
#define SCAN_DDR              DDRC
#define SCAN_PORT             PORTC
#define SCAN_SHIFT            0

// We assume that key columns are 3 consecutive lines in one port
#define KEY_DDR               DDRD
#define KEY_PORT              PORTD
#define KEY_PORTIN            PIND
#define KEY_SHIFT             2

// Display pins
#define DSP_DDR               DDRB
#define DSP_PORT              PORTB
#define DSP_ENABLE            2
#define DSP_CLOCK             5
#define DSP_DATA              3

// Beeper
#define BEEP_DDR              DDRD
#define BEEP_PORT             PORTD
#define BEEP_PIN              5

// LED
#define LED_DDR               DDRC
#define LED_PORT              PORTC
#define LED_PIN               4


// iButton
#define DALLAS_PORT           PORTC
#define DALLAS_DDR            DDRC
#define DALLAS_PORTIN         PINC
#define DALLAS_PIN            5

// SPI
#define SPI_PORT              PORTB
#define SPI_DDR               DDRB
#define SPI_PORTIN            PINB
#define SPI_SCK               5
#define SPI_MISO              4
#define SPI_MOSI              3
#define SPI_SS                2

#endif /* CONFIG_H */
