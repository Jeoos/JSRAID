/*
 * sraid_common.c for the kernel software
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include "libsraid.h"

struct sraid_options *
sraid_parse_options(const char *options, int (*parse_extra_token)(char *c, void *private),
			void *private)
{
        return NULL;
}

void sraid_destroy_options(struct sraid_options *opt)
{
	printk("destroy_options %p\n", opt);
	kfree(opt->name);
	kfree(opt);
}

void sraid_destroy_client(struct sraid_client *client)
{

}

int sraid_compare_options(struct sraid_options *new_opt,
			 struct sraid_client *client)
{
        return 0;
}

struct sraid_client *sraid_create_client(struct sraid_options *opt, void *private,
				       u64 supported_features,
				       u64 required_features)
{
        return NULL;
}

int sraid_open_session(struct sraid_client *client)
{
        return 0;
}
