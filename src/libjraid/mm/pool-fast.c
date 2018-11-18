/*
 * pool-fast.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <malloc.h>
#include <pthread.h>
#include "libjraid.h" 

static JD_LIST_INIT(_jd_pools);
static pthread_mutex_t _jd_pools_mutex = PTHREAD_MUTEX_INITIALIZER;

struct chunk {
	char *begin, *end;
	struct chunk *prev;
} __attribute__((aligned(8)));

struct jd_pool {
	struct jd_list list;
	struct chunk *chunk, *spare_chunk;
	const char *name;
	size_t chunk_size;
	size_t object_len;
	unsigned object_alignment;
	int locked;
	long crc;
};

struct jd_pool *jraid_pool_create(const char *name, size_t chunk_hint)
{
	size_t new_size = 1024;
        struct jd_pool *p = malloc(sizeof(*p));
        if(!p) {
		printf("Couldn't create memory pool.\n");
		return NULL;
        }
	p->name = name;
	/* round chunk_hint up to the next power of 2 */
	p->chunk_size = chunk_hint + sizeof(struct chunk);
	while (new_size < p->chunk_size)
		new_size <<= 1;
	p->chunk_size = new_size;
	pthread_mutex_lock(&_jd_pools_mutex);
	jd_list_add(&_jd_pools, &p->list);
	pthread_mutex_unlock(&_jd_pools_mutex);
        return p;
}

void jraid_pool_destroy(struct jd_pool *p)
{

}
