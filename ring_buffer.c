// SPDX-License-Identifier: BSD-3-Clause

#include "ring_buffer.h"
#include "packet.h"

int ring_buffer_init(so_ring_buffer_t *rb, size_t cap)
{
	rb->data = malloc(cap);
	if (!rb->data)
		return -1;

	rb->read_pos = 0;
	rb->write_pos = 0;
	rb->len = 0;
	rb->cap = cap;
	rb->stopped = 0;
	rb->v = malloc(cap * sizeof(int));
	rb->k = 0;

	pthread_mutex_init(&rb->mutex, NULL);
	pthread_cond_init(&rb->not_full, NULL);
	pthread_cond_init(&rb->not_empty, NULL);

	return 0;
}

ssize_t ring_buffer_enqueue(so_ring_buffer_t *rb, void *data, size_t size)
{
	pthread_mutex_lock(&rb->mutex);

	while (rb->len + size > rb->cap && !rb->stopped)
		pthread_cond_wait(&rb->not_full, &rb->mutex);

	if (rb->stopped) {
		pthread_mutex_unlock(&rb->mutex);
		return -1;
	}

	memcpy(rb->data + rb->write_pos, data, size);

	rb->write_pos = (rb->write_pos + size) % rb->cap;
	rb->len += size;

	pthread_cond_signal(&rb->not_empty);
	pthread_mutex_unlock(&rb->mutex);
	so_packet_t *packet = (so_packet_t *)data;

	rb->v[rb->k++] = packet->hdr.timestamp;

	return size;
}

ssize_t ring_buffer_dequeue(so_ring_buffer_t *rb, void *data, size_t size)
{
	pthread_mutex_lock(&rb->mutex);

	while (rb->len < size && !rb->stopped)
		pthread_cond_wait(&rb->not_empty, &rb->mutex);

	if (rb->stopped && rb->len < size) {
		pthread_mutex_unlock(&rb->mutex);
		return -1;
	}

	memcpy(data, rb->data + rb->read_pos, size);

	rb->read_pos = (rb->read_pos + size) % rb->cap;
	rb->len -= size;

	pthread_cond_signal(&rb->not_full);
	pthread_mutex_unlock(&rb->mutex);

	return size;
}

void ring_buffer_destroy(so_ring_buffer_t *rb)
{
	free(rb->data);
	pthread_mutex_destroy(&rb->mutex);
	pthread_cond_destroy(&rb->not_full);
	pthread_cond_destroy(&rb->not_empty);
}

void ring_buffer_stop(so_ring_buffer_t *rb)
{
	pthread_mutex_lock(&rb->mutex);
	rb->stopped = 1;
	pthread_cond_broadcast(&rb->not_full);
	pthread_cond_broadcast(&rb->not_empty);
	pthread_mutex_unlock(&rb->mutex);
}
