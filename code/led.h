#ifndef LED_H
#define LED_H

void led_init();
void led_on();
void led_off();
void led_flash();

#ifdef LED_DIGITS
void led_flash_number(int number);
#endif // LED_DIGITS

#endif /* LED_H */
