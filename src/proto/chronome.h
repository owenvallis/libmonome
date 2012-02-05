/*
 *  chronome.h
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

#include "monome.h"
#include "internal.h"

typedef enum {
	/* input (from device) */
	
	PROTO_CHRONOME_PRESSURE            = 0x10,
	
	/* output (to device) */
	
	PROTO_CHRONOME_LED_ON              = 0x02,
	PROTO_CHRONOME_LED_OFF             = 0x03,
	PROTO_CHRONOME_LED_COLOR           = 0x04,	
	PROTO_CHRONOME_LED_ALL_ON          = 0x05,
    PROTO_CHRONOME_LED_ROW             = 0x06,
    PROTO_CHRONOME_LED_COL             = 0x07
    
} proto_chronome_message_t;

typedef struct monome_chronome monome_chronome_t;

struct monome_chronome {
	monome_t parent;
	monome_mode_t mode;
};


