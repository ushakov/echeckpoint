#include "global.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "lcd.h"
#include "misc.h"
#include "delay.h"

const uint8_t A0 = (1 << DDC6);
const uint8_t RW = (1 << DDC5);
const uint8_t E  = (1 << DDC4);
const uint8_t D7 = (1 << DDC3);
const uint8_t D6 = (1 << DDC2);
const uint8_t D5 = (1 << DDC1);
const uint8_t D4 = (1 << DDC0);

const uint8_t DATA = (1 << DDC3) | (1 << DDC2) | (1 << DDC1) | (1 << DDC0);

#define LCD_OUT PORTC
#define LCD_IN PINC
#define LCD_DDR DDRC

static void lcd_prepare_write ()
{
    LCD_DDR |= DATA;
}

static void lcd_prepare_read ()
{
    LCD_DDR &= ~DATA;
    LCD_OUT &= ~DATA;
}

static uint8_t lcd_read_internal(int a0)
{
    uint8_t ret;
    lcd_prepare_read ();
    LCD_OUT = RW;
    if (a0) LCD_OUT |= A0;
    nop();
    LCD_OUT |= E;
    nop(); nop();
    ret = (LCD_IN & DATA) << 4;
    LCD_OUT &= ~E;
    nop(); nop();
    LCD_OUT |= E;
    nop(); nop();
    ret |= LCD_IN & DATA;
    LCD_OUT &= ~E;
    nop(); nop();
    return ret;
}

static void lcd_wait_status() {
    uint8_t r = lcd_read_internal(0);
    while ((r & 0x80) != 0) {
	r = lcd_read_internal(0);
    }
}

static void lcd_write_command_internal (uint8_t data, int a0)
{
    lcd_prepare_write ();
    LCD_OUT = 0;
    if (a0) LCD_OUT |= A0;
    nop();
    LCD_OUT |= ((data >> 4) & DATA);
    nop();
    LCD_OUT |= E;
    nop(); nop();
    LCD_OUT &= ~E;
    nop(); nop();
    LCD_OUT &= ~DATA;
    LCD_OUT |= (data & DATA);
    nop();
    LCD_OUT |= E;
    nop(); nop();
    LCD_OUT &= ~E;
    nop(); nop();
}


void lcd_write_command (uint8_t data)
{
    lcd_wait_status();
    lcd_write_command_internal (data, 0);
}

uint8_t lcd_read_status ()
{
    return lcd_read_internal (0);
}

void lcd_write_data (uint8_t data)
{
    lcd_wait_status();
    lcd_write_command_internal (data, 1);
}

uint8_t lcd_read_data ()
{
    uint8_t ret;
    lcd_wait_status();
    ret = lcd_read_internal (1);
    return ret;
}


const int dms = 50;
const int dus = 100;

void lcd_setup()
{
    LCD_DDR = A0 | RW | E;
    lcd_prepare_write();
    LCD_OUT = 0;
    delay_ms (dms);

    LCD_OUT = D5 | D4;
    nop();
    LCD_OUT |= E;
    nop(); nop();
    LCD_OUT &= ~E;
    _delay_us (dus);

    LCD_OUT |= E;
    nop(); nop();
    LCD_OUT &= ~E;
    _delay_us (dus);

    LCD_OUT |= E;
    nop(); nop();
    LCD_OUT &= ~E;
    _delay_us (dus);
    
    LCD_OUT = D5;
    nop();
    LCD_OUT |= E;
    nop(); nop();
    LCD_OUT &= ~E;
    _delay_us (dus);

    lcd_write_command (0x2a);
    lcd_write_command (0x08);
    lcd_write_command (0x01);
    lcd_write_command (0x06);

    lcd_write_command (0x0c);
}

static int cursor = 0;

void lcd_clear()
{
    lcd_write_data (0x01);
    cursor = 0;
}

static void shift() {
    static char second_line[16];

    lcd_write_command (0x80 + 0x40);
    for (int i = 0; i < 16; ++i) {
	second_line[i] = lcd_read_data ();
    }
    lcd_write_command (0x80);
    for (int i = 0; i < 16; ++i) {
	lcd_write_data (second_line[i]);
    }
    lcd_write_command (0x80 + 0x40);
    for (int i = 0; i < 16; ++i) {
	lcd_write_data (' ');
    }
}

void lcd_log_char (uint8_t data)
{
    if (cursor >= 32) {
	cursor = 16;
	shift();
    }
	
    if (cursor < 16) {
	lcd_write_command (0x80 + cursor);
	lcd_write_data (data);
	cursor++;
    } else {
	lcd_write_command (0x80 + 0x40 + cursor - 16);
	lcd_write_data (data);
	cursor++;
    }
}

void lcd_log_line (uint8_t * line)
{
    for (int i = 0; line[i]; ++i) {
	lcd_log_char (line[i]);
    }
}

void lcd_log_num (int number) {
    int highest = 1;
    while (highest * 10 <= number) highest *= 10;
    while (highest >= 1) {
	lcd_log_char ('0' + number / highest);
	number %= highest;
	highest /= 10;
    }
}

void lcd_log_numx (int number) {
    int highest = 1;
    while (highest * 16 <= number) highest *= 16;
    while (highest >= 1) {
	int digit = number / highest;
	lcd_log_char ((digit < 10) ? ('0' + digit) : ('a' + digit - 10));
	number %= highest;
	highest /= 16;
    }
}
    

void lcd_set_str (uint8_t * str)
{
    lcd_write_command (0x80);
    for (int i = 0; i < 16 && str[i]; ++i)
    {
	lcd_write_data (str[i]);
    }
}

void lcd_set_str2 (uint8_t * str)
{
    lcd_write_command (0x80 + 0x40);
    for (int i = 0; i < 16 && str[i]; ++i)
    {
	lcd_write_data (str[i]);
    }
}
