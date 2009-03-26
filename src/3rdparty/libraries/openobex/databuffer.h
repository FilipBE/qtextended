/*************************************************************************
 *
 * Filename:      databuffer.h
 * Version:       0.3
 * Description:   Network buffer handling routines.
 * Status:        Experimental.
 * Author:        Herton Ronaldo Krzesinski <herton@conectiva.com.br>
 * Created at:    Tue Jun 21 11:22:44 2005
 * Modified at:   Mon Aug 15 19:12:29 2005
 *
 *     Copyright (c) 2005 Herton Ronaldo Krzesinski, All Rights Reserved.
 *
 *     This library is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU Lesser General Public
 *     License as published by the Free Software Foundation; either
 *     version 2 of the License, or (at your option) any later version.
 *
 *     This library is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *     Lesser General Public License for more details.
 *
 *     You should have received a copy of the GNU Lesser General Public
 *     License along with this library; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *     MA  02111-1307  USA
 *
 *************************************************************************/

#ifndef DATABUFFER_H
#define DATABUFFER_H

#define __need_size_t
#include <stddef.h>
#include <stdint.h>

/*
 * Implements a single linked list
 */
typedef struct _slist_t {
        void *data;
        struct _slist_t *next;
} slist_t;

typedef struct {
	uint8_t *buffer; // buffer containing the allocated space
	uint8_t *data; // pointer to the location of *buffer where there is valid data
	size_t head_avail; // number of allocated space available in front of buffer
	size_t data_avail; // allocated space available not specific for head or tail
	size_t tail_avail; // number of allocated space available at end of buffer
	size_t data_size; // number of allocated space used
} buf_t;

buf_t *buf_new(size_t default_size);
size_t buf_total_size(buf_t *p);
void buf_resize(buf_t *p, size_t new_size);
buf_t *buf_reuse(buf_t *p);
void *buf_reserve_begin(buf_t *p, size_t data_size);
void *buf_reserve_end(buf_t *p, size_t data_size);
void buf_insert_begin(buf_t *p, uint8_t *data, size_t data_size);
void buf_insert_end(buf_t *p, uint8_t *data, size_t data_size);
void buf_remove_begin(buf_t *p, size_t data_size);
void buf_remove_end(buf_t *p, size_t data_size);
void buf_dump(buf_t *p, const char *label);
void buf_free(buf_t *p);

slist_t *slist_append(slist_t *list, void *element);
slist_t *slist_remove(slist_t *list, void *element);

#endif
