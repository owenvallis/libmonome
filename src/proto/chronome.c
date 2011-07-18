/*
 *  chronome.c
 *  
 *
 * Created by owen vallis on 5/27/11.
 * Copyright (c) 2011 Flipmu <contact@flipmu.com>
 *
 * Based on work that is Copyright (c) 2010 William Light <wrl@illest.net>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <monome.h>
#include "internal.h"
#include "platform.h"
#include "rotation.h"

#include "chronome.h"

/**
 * private
 */

static int monome_write(monome_t *monome, const uint8_t *buf,
						ssize_t bufsize) {
    if( monome_platform_write(monome, buf, bufsize) == bufsize )
		return 0;
	
	return -1;
}

/**
 * public led functions
 */

static int proto_chronome_led_all(monome_t *monome, uint_t status) {
    
	uint8_t buf[1];
	
	buf[0] = 0x80 | (PROTO_CHRONOME_LED_ALL_ON | (status << 4));
	
	return monome_write(monome, buf, sizeof(buf));
	
}

// intensity is dealt with using the color function
static int proto_chronome_intensity(monome_t *monome, uint_t brightness) {
    
    return 0;
}

// the chronome has no mode function as it doesn't use the MAX7219
static int proto_chronome_mode(monome_t *monome, monome_mode_t mode) {
    
	return 0;
}

static int proto_chronome_led_set(monome_t *monome, uint_t x, uint_t y, uint_t on) {
    uint8_t buf[2];

	ROTATE_COORDS(monome, x, y);
    
    x &= 0x7;
	y &= 0x7;
	
	buf[0] = 0x80 | (PROTO_CHRONOME_LED_ON + !on);
	buf[1] = 0x7F & ((x << 4) | y);
	
	return monome_write(monome, buf, sizeof(buf));
}

static int proto_chronome_led_col(monome_t *monome, uint_t x, uint_t y_off,
                             size_t count, const uint8_t *data) {
    int ret = 0;
    uint8_t state;
    
    /*
    uint8_t rowStates;
    
    if( ROTSPEC(monome).flags & COL_REVBITS )
        rowStates = REVERSE_BYTE(*data);
    else
        rowStates = *data;
    */
    for(int y = 0; y < 8; y++) {
        state = (*data >> y) & 0x01;
        ret += proto_chronome_led_set(monome, x, y, state);
    }
    
    return ret;
}

static int proto_chronome_led_row(monome_t *monome, uint_t x_off, uint_t y,
                             size_t count, const uint8_t *data) {
    int ret = 0;
    uint8_t state;
    /*
     uint8_t rowStates;
     
     if( ROTSPEC(monome).flags & ROW_REVBITS )
     rowStates = REVERSE_BYTE(*data);
     else
     rowStates = *data;
     */
    for(int x = 0; x < 8; x++) {
        state = (*data >> x) & 0x01;
        ret += proto_chronome_led_set(monome, x, y, state);
    }
    
    return ret;
}

static int proto_chronome_led_map(monome_t *monome, uint_t x_off, uint_t y_off,
                             const uint8_t *data) {
    return 0;
}

static int proto_chronome_led_color(monome_t *monome, uint_t x, uint_t y,
								    uint_t r, uint_t g, uint_t b)
{
	
	uint8_t buf[5];
	
	buf[0] = 0x80 | PROTO_CHRONOME_LED_COLOR;
	buf[1] = 0x7F & ((x << 4) | y);
	
	buf[2] = 0x7F & (uint8_t)r;
	buf[3] = 0x7F & (uint8_t)g;
	buf[4] = 0x7F & (uint8_t)b;
	
	return monome_write(monome, buf, sizeof(buf));
}

static monome_led_functions_t proto_chronome_led_functions = {
	.set        = proto_chronome_led_set,
	.color      = proto_chronome_led_color,
	.all        = proto_chronome_led_all,
    .map        = proto_chronome_led_map,
	.row        = proto_chronome_led_row,
	.col        = proto_chronome_led_col,
    .intensity  = proto_chronome_intensity
};


static int proto_chronome_next_event(monome_t *monome, monome_event_t *e) {
	uint8_t buf[3] = {0, 0, 0};
	
	if( monome_platform_read(monome, buf, sizeof(buf)) != sizeof(buf) )
		return 0;
	
	//outputHex((const uint8_t*) &buf, (ssize_t) 2);
	
	switch( buf[0] & 0xF0 ) {			
		case PROTO_CHRONOME_PRESSURE:
			e->event_type = MONOME_PRESSURE;
			
			e->pressure.x = buf[0] & 0x0F;
			e->pressure.y = buf[1] >> 4;            
			e->pressure.value  = ((buf[1] & 0x03) << 8) | buf[2];
            
            UNROTATE_COORDS(monome, e->pressure.x, e->pressure.y);
            
            if(chronomePreviousValue[e->pressure.x][e->pressure.y] == 0 && e->pressure.value > 0) {
                e->event_type = MONOME_BUTTON_DOWN;
                e->grid.x = e->pressure.x;
                e->grid.y = e->pressure.y;
            }
            else if(chronomePreviousValue[e->pressure.x][e->pressure.y] > 0 && e->pressure.value == 0) {
                e->event_type = MONOME_BUTTON_UP;
                e->grid.x = e->pressure.x;
                e->grid.y = e->pressure.y;
            }
            
            chronomePreviousValue[e->pressure.x][e->pressure.y] = e->pressure.value;
            
			return 1;
	}
	
	return 0;
}

static int proto_chronome_open(monome_t *monome, const char *dev,
							   const char *serial, const monome_devmap_t *m,
							   va_list args) {
	monome->rows   = m->dimensions.rows;
	monome->cols   = m->dimensions.cols;
	monome->serial = serial;
	monome->friendly = m->friendly;
	
	return monome_platform_open(monome, m, dev);
}

static int proto_chronome_close(monome_t *monome) {
	return monome_platform_close(monome);
}

static void proto_chronome_free(monome_t *monome) {
	m_free(monome);
}

monome_t *monome_protocol_new() {
	monome_t *monome = m_calloc(1, sizeof(monome_t));
	
	if( !monome )
		return NULL;
	
	monome->open       = proto_chronome_open;
	monome->close      = proto_chronome_close;
	monome->free       = proto_chronome_free;
	
	monome->next_event = proto_chronome_next_event;
	
	monome->mode       = proto_chronome_mode;
	
	monome->led        = &proto_chronome_led_functions;
	monome->led_level  = NULL;
	monome->led_ring   = NULL;
	
	return monome;
}