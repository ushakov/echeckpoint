#ifndef LCD_H
#define LCD_H

#include <stdint.h>

void lcd_write_command (uint8_t data);
uint8_t lcd_read_status ();
void lcd_write_data (uint8_t data);
uint8_t lcd_read_data ();
void lcd_setup();

// ----- high level

void lcd_clear();
void lcd_log_char(uint8_t data);
void lcd_log_line(uint8_t * line);
void lcd_log_num(int number);
void lcd_log_numx (int number);
void lcd_set_str (uint8_t * str);
void lcd_set_str2 (uint8_t * str);


#endif /* LCD_H */
