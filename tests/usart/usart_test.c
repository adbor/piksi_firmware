/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/f4/rcc.h>
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/f4/nvic.h>

#include "main.h"
#include "board/leds.h"
#include "error.h"

void clock_setup(void)
{
  /* Enable GPIOD clock for LED & USARTs. */
  rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPCEN);

  /* Enable clocks for USART6. */
  rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_USART6EN);
}

void usart_setup(void)
{
  /* Setup USART2 parameters. */
  usart_set_baudrate(USART6, 1000000);
  usart_set_databits(USART6, 8);
  usart_set_stopbits(USART6, USART_STOPBITS_1);
  usart_set_mode(USART6, USART_MODE_TX_RX);
  usart_set_parity(USART6, USART_PARITY_NONE);
  usart_set_flow_control(USART6, USART_FLOWCONTROL_NONE);

  /* Finally enable the USART. */
  usart_enable(USART6);
}

void gpio_setup(void)
{
  gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6|GPIO7);
  gpio_set_af(GPIOC, GPIO_AF8, GPIO6|GPIO7);
}

int main(void)
{
  clock_setup();
  gpio_setup();
  usart_setup();
  nvic_enable_irq(NVIC_USART6_IRQ);

  led_setup();

  led_on(LED_RED);
  led_off(LED_GREEN);

  /* Blink the LED on the board with every transmitted byte. */
  char c = 0;
  while (1) {
    led_toggle(LED_RED);
    led_toggle(LED_GREEN);
    for (int i = 0; i < 100000; i++) /* Wait a bit. */
      __asm__("NOP");
    DO_EVERY(26,
      for (int i = 0; i < 100000; i++) /* Wait a bit. */
        __asm__("NOP");
      USART6_DR = '\n';
      for (int i = 0; i < 100000; i++) /* Wait a bit. */
        __asm__("NOP");
      USART6_DR = '\r';
    );
    USART6_DR = c + 'A';
    c = (c + 1) % 26;
  }

  return 0;
}
