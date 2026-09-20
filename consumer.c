// SPDX-License-Identifier: BSD-3-Clause

#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

#include "consumer.h"
#include "ring_buffer.h"
#include "packet.h"
#include "utils.h"

void *consumer_thread(so_consumer_ctx_t *ctx)
{
	/* TODO: implement consumer thread */
	char outbuf[PKT_SZ];

	while (1) {
		so_packet_t packet;
		int result = ring_buffer_dequeue(ctx->producer_rb, &packet, PKT_SZ);

		if (result < 0)
			break;
		int action = process_packet(&packet);
		unsigned long hash = packet_hash(&packet);
		unsigned long timestamp = packet.hdr.timestamp;

		pthread_mutex_lock(&ctx->lock);
		while (ctx->producer_rb->v[ctx->position] != (int)timestamp)
			pthread_cond_wait(&ctx->cond, &ctx->lock);
		int len = snprintf(outbuf, 256, "%s %016lx %lu\n",
			RES_TO_STR(action), hash, timestamp);
		write(ctx->fd, outbuf, len);
		ctx->position++;
		pthread_cond_broadcast(&ctx->cond);
		pthread_mutex_unlock(&ctx->lock);
	}
	pthread_exit(NULL);
	return NULL;
}

int create_consumers(pthread_t *tids,
					 int num_consumers,
					 struct so_ring_buffer_t *rb,
					 const char *out_filename)
{
	(void) tids;
	(void) num_consumers;
	(void) rb;
	(void) out_filename;
	so_consumer_ctx_t *ctx = malloc(sizeof(so_consumer_ctx_t));

	ctx->producer_rb = rb;
	ctx->position = 0;
	pthread_mutex_init(&ctx->lock, NULL);
	pthread_cond_init(&ctx->cond, NULL);
	int out_fd = open(out_filename, O_RDWR|O_CREAT|O_TRUNC, 0666);

	DIE(out_fd < 0, "open");
	ctx->fd = out_fd;
	for (int i = 0; i < num_consumers; i++) {
		/*
		 * TODO: Launch consumer threads
		 **/
		pthread_create(&tids[i], NULL, (void * (*)(void *))consumer_thread, ctx);
	}
	return num_consumers;
}
