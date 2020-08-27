/*
 * hagl_util.h
 *
 *  Created on: Aug 26, 2020
 *      Author: allen
 */
#ifndef MAIN_HAGL_UTIL_H_
#define MAIN_HAGL_UTIL_H_

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "hagl_hal.h"
#include "hagl.h"
#include "math.h"


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef struct hagl_driver
{
	int pin_reset;
		//int pin_dc;
		//int pin_mosi;
		//int pin_sclk;
		//int spi_host;
		//int dma_chan;
		uint8_t queue_fill;
		uint16_t display_width;
		uint16_t display_height;
		//spi_device_handle_t spi;
		size_t buffer_size;
		//st7789_transaction_data_t data;
		//st7789_transaction_data_t command;
		color_t *buffer;
		color_t *buffer_a;
		color_t *buffer_b;
		color_t *current_buffer;
		//spi_transaction_t trans_a;
		//spi_transaction_t trans_b;
} hagl_driver_t;

void hagl_write_pixels(hagl_driver_t *driver, color_t *pixels, size_t length);
void hagl_swap_buffers(hagl_driver_t *driver);

extern uint8_t hagl_dither_table[];
void hagl_randomize_dither_table();
#define hagl_rgb_to_color(r, g, b) ((((color_t)(r) >> 3) << 11) | (((color_t)(g) >> 2) << 5) | ((color_t)(b) >> 3))
inline color_t __attribute__((always_inline)) hagl_rgb_to_color_dither(uint8_t r, uint8_t g, uint8_t b, uint16_t x, uint16_t y) {
	const uint8_t pos = ((y << 8) + (y << 3) + x) & 0xff;
	uint8_t rand_b = hagl_dither_table[pos];
	const uint8_t rand_r = rand_b & 0x07;
	rand_b >>= 3;
	const uint8_t rand_g = rand_b & 0x03;
	rand_b >>= 2;

	if (r < 249) {
		r = r + rand_r;
	}
	if (g < 253) {
		g = g + rand_g;
	}
	if (b < 249) {
		b = b + rand_b;
	}
	return hagl_rgb_to_color(r, g, b);
}


inline void __attribute__((always_inline)) hagl_color_to_rgb(color_t color, uint8_t *r, uint8_t *g, uint8_t *b) {
	*b = (color << 3);
	color >>= 5;
	color <<= 2;
	*g = color;
	color >>= 8;
	*r = color << 3;
}
void hagl_draw_gray2_bitmap(uint8_t *src_buf, color_t *target_buf, uint8_t r, uint8_t g, uint8_t b, int x, int y, int src_w, int src_h, int target_w, int target_h);


#endif /* MAIN_HAGL_UTIL_H_ */
