/*
 * bcache.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __BCACHE_H__
#define __BCACHE_H__

#include "../../include/libjraid.h"
#include "../../include/jdstruct.h"

#include <linux/fs.h>
#include <stdint.h>
#include <stdbool.h>

/* FIXME: move somewhere more sensible */
#define container_of(v, t, head) \
    ((t *)((const char *)(v) - (const char *)&((t *) 0)->head))

enum dir {
	DIR_READ,
	DIR_WRITE
};

typedef uint64_t block_address;
typedef uint64_t sector_t;

typedef void io_complete_fn(void *context, int io_error);

struct io_engine {
	void (*destroy)(struct io_engine *e);
	bool (*issue)(struct io_engine *e, enum dir d, int fd,
		      sector_t sb, sector_t se, void *data, void *context);
	bool (*wait)(struct io_engine *e, io_complete_fn fn);
	unsigned (*max_io)(struct io_engine *e);
};

struct io_engine *create_async_io_engine(void);
struct io_engine *create_sync_io_engine(void);

struct bcache;
struct block {
	/* clients may only access these three fields */
	int fd;
	uint64_t index;
	void *data;

	struct bcache *cache;
	struct jd_list list;
	struct jd_list hash;

	unsigned flags;
	unsigned ref_count;
	int error;
	enum dir io_dir;
};

/*
 * Ownership of engine passes.  Engine will be destroyed even if this fails.
 */
struct bcache *bcache_create(sector_t block_size, unsigned nr_cache_blocks,
			     struct io_engine *engine);
void bcache_destroy(struct bcache *cache);

enum bcache_get_flags {
	/*
	 * The block will be zeroed before get_block returns it.  This
	 * potentially avoids a read if the block is not already in the cache.
	 * GF_DIRTY is implicit.
	 */
	GF_ZERO = (1 << 0),

	/*
	 * Indicates the caller is intending to change the data in the block, a
	 * writeback will occur after the block is released.
	 */
	GF_DIRTY = (1 << 1)
};

sector_t bcache_block_sectors(struct bcache *cache);
unsigned bcache_nr_cache_blocks(struct bcache *cache);
unsigned bcache_max_prefetches(struct bcache *cache);

/*
 * Use the prefetch method to take advantage of asynchronous IO.  For example,
 * if you wanted to read a block from many devices concurrently you'd do
 * something like this:
 *
 * jd_list_iterate_items (dev, &devices)
 * 	bcache_prefetch(cache, dev->fd, block);
 *
 * jd_list_iterate_items (dev, &devices) {
 *	if (!bcache_get(cache, dev->fd, block, &b))
 *		fail();
 *
 *	process_block(b);
 * }
 *
 * It's slightly sub optimal, since you may not run the gets in the order that
 * they complete.  But we're talking a very small difference, and it's worth it
 * to keep callbacks out of this interface.
 */
void bcache_prefetch(struct bcache *cache, int fd, block_address index);

/*
 * Returns true on success.
 */
bool bcache_get(struct bcache *cache, int fd, block_address index,
	        unsigned flags, struct block **result);
void bcache_put(struct block *b);

/*
 * flush() does not attempt to writeback locked blocks.  flush will fail
 * (return false), if any unlocked dirty data cannot be written back.
 */
bool bcache_flush(struct bcache *cache);

/*
 * Removes a block from the cache.
 * 
 * If the block is dirty it will be written back first.  If the writeback fails
 * false will be returned.
 * 
 * If the block is currently held false will be returned.
 */
bool bcache_invalidate(struct bcache *cache, int fd, block_address index);

/*
 * Invalidates all blocks on the given descriptor.  Call this before closing
 * the descriptor to make sure everything is written back.
 */
bool bcache_invalidate_fd(struct bcache *cache, int fd);


/* 
 * The next four functions are utilities written in terms of the above api. 
 */
 
/* prefetches the blocks neccessary to satisfy a byte range. */
void bcache_prefetch_bytes(struct bcache *cache, int fd, uint64_t start, size_t len);

/* reads, writes and zeroes bytes.  Returns false if errors occur. */
bool bcache_read_bytes(struct bcache *cache, int fd, uint64_t start, size_t len, void *data);
bool bcache_write_bytes(struct bcache *cache, int fd, uint64_t start, size_t len, void *data);
bool bcache_zero_bytes(struct bcache *cache, int fd, uint64_t start, size_t len);
bool bcache_set_bytes(struct bcache *cache, int fd, uint64_t start, size_t len, uint8_t val);

#endif
