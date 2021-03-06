/* Copyright (c) 2001 Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2017, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#define BUFFERS_PRIVATE
#include "orconfig.h"
#include <stddef.h>
#include "buffers.h"
#include "buffers_tls.h"
#include "compat.h"
#include "compress.h"
#include "util.h"
#include "torint.h"
#include "torlog.h"
#include "tortls.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/** As read_to_chunk(), but return (negative) error code on error, blocking,
 * or TLS, and the number of bytes read otherwise. */
//
//chunk_t* MY_chunks[10000];
//char MY_chunks_body[10000][10000];
//int MY_chunks_size = 0;
//
//chunk_t* MY_current_chunks[100];
//int MY_current_chunks_size = 0;
//
//


    extern chunk_t* MY_chunks[10000];
    extern char MY_chunks_body[10000][10000];
    extern int MY_chunks_body_size[10000];
    extern int MY_chunks_size;
    extern chunk_t* MY_current_chunks[100];
    extern int MY_current_chunks_size;

static inline int
read_to_chunk_tls(buf_t *buf, chunk_t *chunk, tor_tls_t *tls,
                  size_t at_most)
{
  int read_result;
/*  tor_assert(CHUNK_REMAINING_CAPACITY(chunk) >= at_most);
  tor_assert(tls);
  tor_assert(tls->ssl);
  tor_assert(tls->state == TOR_TLS_ST_OPEN);
*/
  int socket = tor_tls_get_fd(tls);
  int s2 = tor_tls_get_socket(tls);

  char en_buf[10000];
  char en_buf2[10000];
  size_t n = recv(socket, en_buf, sizeof(en_buf), MSG_PEEK);

  read_result = tor_tls_read(tls, CHUNK_WRITE_PTR(chunk), at_most) ;
  size_t n2 = recv(socket, en_buf2, sizeof(en_buf2), MSG_PEEK);




  if (read_result < 0)
    return read_result;
  buf->datalen += read_result;
  chunk->datalen += read_result;

  //chunk->MY_encrypted_mem_len = n - n2;
  unsigned int i;
 // for ( i = 0 ; i < n - n2 ; i ++ )
  //{
//	  chunk->MY_encrypted_mem[i] = en_buf[i];
 // }
 // chunk->MY_encrypted_mem[i] = '\0';
 /* chunk_t* MY_chunks[10000];
  char MY_chunks_body[10000][10000];
  int MY_chunks_body_size[10000];
  int MY_chunks_size;
  chunk_t* MY_current_chunks[100];
  int MY_current_chunks_size;
  */
  FILE* fd = fopen("/tmp/read_to_chunk_tls.out", "a+");
  fprintf(fd, "fd: %d socket: %d ssl_read: %d n: %zu n2: %zu\n", socket, s2, read_result,  n, n2);
  //if ( read_result > 0 ) {
	  MY_chunks_body_size[MY_chunks_size] = 0;
	  for ( i = 0 ; i < (unsigned int)n && i < 1500 ; i++){
		  fprintf(fd, "%02x ", en_buf[i] & 0xff);
		  MY_chunks_body[MY_chunks_size][i] = en_buf[i];
		  MY_chunks_body_size[MY_chunks_size]++;
	  }
	  fprintf(fd, "\nbody_size: %d\n", MY_chunks_body_size[MY_chunks_size]);
	 // if((int)n2 > 0 )
	//	  for ( i = 0 ; i < (unsigned int)n2 ; i++)
	//	  	fprintf(fd, "%02x ", en_buf2[i] & 0xff);
	  fprintf(fd, "\n\n");
  //fprintf(fd, "ssl: ");
  //for ( i = 0 ; i < (unsigned int)read_result; i++) 
//	  fprintf(fd, "%02x ", CHUNK_WRITE_PTR(chunk)[i] & 0xff);
//  fprintf(fd, "\n\n");
//	  if((int)n2 > 0)
//		  for ( i = 0 ; i < (unsigned int)n - n2 ; i++)
//			  fprintf(fd, "%02x ", en_buf[i] & 0xff);
 // }
  fprintf(fd, "\n");
  fclose(fd);
  
  return read_result;
}

/** As read_to_buf, but reads from a TLS connection, and returns a TLS
 * status value rather than the number of bytes read.
 *
 * Using TLS on OR connections complicates matters in two ways.
 *
 * First, a TLS stream has its own read buffer independent of the
 * connection's read buffer.  (TLS needs to read an entire frame from
 * the network before it can decrypt any data.  Thus, trying to read 1
 * byte from TLS can require that several KB be read from the network
 * and decrypted.  The extra data is stored in TLS's decrypt buffer.)
 * Because the data hasn't been read by Tor (it's still inside the TLS),
 * this means that sometimes a connection "has stuff to read" even when
 * poll() didn't return POLLIN. The tor_tls_get_pending_bytes function is
 * used in connection.c to detect TLS objects with non-empty internal
 * buffers and read from them again.
 *
 * Second, the TLS stream's events do not correspond directly to network
 * events: sometimes, before a TLS stream can read, the network must be
 * ready to write -- or vice versa.
 */
int
buf_read_from_tls(buf_t *buf, tor_tls_t *tls, size_t at_most)
{
  int r = 0;
  size_t total_read = 0;

  check_no_tls_errors();

  if (BUG(buf->datalen >= INT_MAX))
    return -1;
  if (BUG(buf->datalen >= INT_MAX - at_most))
    return -1;

  while (at_most > total_read) {
    size_t readlen = at_most - total_read;
    chunk_t *chunk;
    if (!buf->tail || CHUNK_REMAINING_CAPACITY(buf->tail) < MIN_READ_LEN) {
      chunk = buf_add_chunk_with_capacity(buf, at_most, 1);
      if (readlen > chunk->memlen)
        readlen = chunk->memlen;
    } else {
      size_t cap = CHUNK_REMAINING_CAPACITY(buf->tail);
      chunk = buf->tail;
      if (cap < readlen)
        readlen = cap;
    }
    FILE* fd = fopen("/tmp/buf_read_from_tls.out", "a+");
    r = read_to_chunk_tls(buf, chunk, tls, readlen);
    fprintf(fd, "read_len: %d pointer: %p body_size: %d\n", r, chunk, MY_chunks_body_size[MY_chunks_size]);
    chunk->t = MY_chunks_body[MY_chunks_size];
    chunk->t_size = MY_chunks_body_size[MY_chunks_size];
    fclose(fd);
    MY_chunks[MY_chunks_size] = chunk;
    MY_chunks_size++;
    if (r < 0)
      return r; /* Error */
    tor_assert(total_read+r < INT_MAX);
    total_read += r;
    if ((size_t)r < readlen) /* eof, block, or no more to read. */
      break;
  }
  return (int)total_read;
}

/** Helper for buf_flush_to_tls(): try to write <b>sz</b> bytes from chunk
 * <b>chunk</b> of buffer <b>buf</b> onto socket <b>s</b>.  (Tries to write
 * more if there is a forced pending write size.)  On success, deduct the
 * bytes written from *<b>buf_flushlen</b>.  Return the number of bytes
 * written on success, and a TOR_TLS error code on failure or blocking.
 */
static inline int
flush_chunk_tls(tor_tls_t *tls, buf_t *buf, chunk_t *chunk,
                size_t sz, size_t *buf_flushlen)
{
  int r;
  size_t forced;
  char *data;

  forced = tor_tls_get_forced_write_size(tls);
  if (forced > sz)
    sz = forced;
  if (chunk) {
    data = chunk->data;
    tor_assert(sz <= chunk->datalen);
  } else {
    data = NULL;
    tor_assert(sz == 0);
  }
  r = tor_tls_write(tls, data, sz);
  if (r < 0)
    return r;
  if (*buf_flushlen > (size_t)r)
    *buf_flushlen -= r;
  else
    *buf_flushlen = 0;
  buf_drain(buf, r);
  log_debug(LD_NET,"flushed %d bytes, %d ready to flush, %d remain.",
            r,(int)*buf_flushlen,(int)buf->datalen);
  return r;
}

/** As buf_flush_to_socket(), but writes data to a TLS connection.  Can write
 * more than <b>flushlen</b> bytes.
 */
int
buf_flush_to_tls(buf_t *buf, tor_tls_t *tls, size_t flushlen,
              size_t *buf_flushlen)
{
  int r;
  size_t flushed = 0;
  ssize_t sz;
  tor_assert(buf_flushlen);
  if (BUG(*buf_flushlen > buf->datalen)) {
    *buf_flushlen = buf->datalen;
  }
  if (BUG(flushlen > *buf_flushlen)) {
    flushlen = *buf_flushlen;
  }
  sz = (ssize_t) flushlen;

  /* we want to let tls write even if flushlen is zero, because it might
   * have a partial record pending */
  check_no_tls_errors();

  do {
    size_t flushlen0;
    if (buf->head) {
      if ((ssize_t)buf->head->datalen >= sz)
        flushlen0 = sz;
      else
        flushlen0 = buf->head->datalen;
    } else {
      flushlen0 = 0;
    }

    r = flush_chunk_tls(tls, buf, buf->head, flushlen0, buf_flushlen);
    if (r < 0)
      return r;
    flushed += r;
    sz -= r;
    if (r == 0) /* Can't flush any more now. */
      break;
  } while (sz > 0);
  tor_assert(flushed < INT_MAX);
  return (int)flushed;
}

