/*
 * hagl_util.c
 *
 *  Created on: Aug 26, 2020
 *      Author: allen
 */
#include "hagl_util.h"
#include "hagl_hal.h"
#include "esp_log.h"

static const char *TAG = "hagl_util";

void hagl_write_pixels(hagl_driver_t *driver, color_t *pixels, size_t length) {
	ESP_LOGI(TAG, "hagl_write_pixels l=%d", length);
#if 0
	st7789_wait_until_queue_empty(driver);

	spi_transaction_t *trans = driver->current_buffer == driver->buffer_a ? &driver->trans_a : &driver->trans_b;
	memset(trans, 0, sizeof(&trans));
	trans->tx_buffer = driver->current_buffer;
	trans->user = &driver->data;
	trans->length = length * sizeof(st7789_color_t) * 8;
	trans->rxlength = 0;

	spi_device_queue_trans(driver->spi, trans, portMAX_DELAY);
	driver->queue_fill++;
#endif
	bitmap_t src = {
			.width = DISPLAY_WIDTH,
			.height = DISPLAY_HEIGHT,
			.depth = DISPLAY_DEPTH,
			.size = length,
			.buffer = (uint8_t*)driver->current_buffer

	};
	hagl_hal_blit(0, 0,  &src);
}

void hagl_swap_buffers(hagl_driver_t *driver) {
	hagl_write_pixels(driver, driver->current_buffer, driver->buffer_size);
	driver->current_buffer = driver->current_buffer == driver->buffer_a ? driver->buffer_b : driver->buffer_a;
}

uint8_t hagl_dither_table[256] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void hagl_randomize_dither_table() {
	uint16_t *dither_table = (uint16_t *)hagl_dither_table;
	for (size_t i = 0; i < sizeof(hagl_dither_table) / 2; ++i) {
		dither_table[i] = rand() & 0xffff;
	}
}


void hagl_draw_gray2_bitmap(uint8_t *src_buf, color_t *target_buf, uint8_t r, uint8_t g, uint8_t b, int x, int y, int src_w, int src_h, int target_w, int target_h) {
	ESP_LOGI(TAG, "start hagl_draw_gray2_bitmap");
	if (x >= target_w || y >= target_h || x + src_w <= 0 || y + src_h <= 0) {
		return;
	}

	const size_t src_size = src_w * src_h;
	const size_t target_size = target_w * target_h;
	const size_t line_w = MIN(src_w + x, target_w) - MAX(x, 0);
	const size_t src_skip = src_w - line_w;
	const size_t target_skip = target_w - line_w;
	size_t src_pos = 0;
	size_t target_pos = 0;
	size_t x_pos = 0;
	size_t y_pos = 0;

	if (y < 0) {
		src_pos = (-y) * src_w;
	}
	if (x < 0) {
		src_pos -= x;
	}
	if (y > 0) {
		target_pos = y * target_w;
	}
	if (x > 0) {
		target_pos += x;
	}

	while (src_pos < src_size && target_pos < target_size) {
		uint8_t src_r, src_g, src_b;
		uint8_t target_r, target_g, target_b;
		hagl_color_to_rgb(target_buf[target_pos], &src_r, &src_g, &src_b);
		uint8_t gray2_color = (src_buf[src_pos >> 2] >> ((src_pos & 0x03) << 1)) & 0x03;
		/*
		static const uint32_t src_weights = 0x002b5580;
		static const uint32_t target_weights = 0x80552b00;
		const uint32_t src_weight = (src_weights >> (gray2_color << 3)) & 0xff;
		const uint32_t target_weight = (target_weights >> (gray2_color << 3)) & 0xff;
		target_r = ((src_weight * src_r) + (target_weight * r)) >> 7;
		target_g = ((src_weight * src_g) + (target_weight * g)) >> 7;
		target_b = ((src_weight * src_b) + (target_weight * b)) >> 7;
		target_buf[target_pos] = st7789_rgb_to_color_dither(target_r, target_g, target_b, x_pos, y_pos);
		*/
		switch(gray2_color) {
			case 1:
				target_r = r >> 1;
				target_g = g >> 1;
				target_b = b >> 1;
				src_r = (src_r >> 1) + target_r;
				src_g = (src_g >> 1) + target_g;
				src_b = (src_b >> 1) + target_b;
				target_buf[target_pos] = hagl_rgb_to_color_dither(src_r, src_g, src_b, x_pos, y_pos);
				break;
			case 2:
				target_r = r >> 2;
				target_g = g >> 2;
				target_b = b >> 2;
				src_r = (src_r >> 2) + target_r + target_r + target_r;
				src_g = (src_g >> 2) + target_g + target_g + target_g;
				src_b = (src_b >> 2) + target_b + target_b + target_b;
				target_buf[target_pos] = hagl_rgb_to_color_dither(src_r, src_g, src_b, x_pos, y_pos);
				break;
			case 3:
				target_buf[target_pos] = hagl_rgb_to_color_dither(r, g, b, x_pos, y_pos);
				break;
			default:
				break;
		}

		x_pos++;

		if (x_pos == line_w) {
			x_pos = 0;
			y_pos++;
			src_pos += src_skip;
			target_pos += target_skip;
		}
		src_pos++;
		target_pos++;
	}

}
