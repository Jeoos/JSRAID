/*
 * libsraid.h for the kernel software
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LIBSRAID_H__
#define __LIBSRAID_H__

#define SRAID_NOPOOL    ((__u64) (-1)) /* pool id no define yet */

#define SRAID_OPT_NOSHARE          (1<<1) /* don't share client with other sbs */

struct sraid_fsid {
        unsigned char fsid[16];
};

/*
 * Maybe needed, Je vs raid option struct
 * blah blah ...
 */
struct sraid_options {
        int flags;
        struct sraid_fsid fsid;

        char *name;
};

struct sraid_client {
	struct sraid_fsid fsid;
	bool have_fsid;

	void *private;

	struct sraid_options *options;
};

extern struct sraid_options *
sraid_parse_options(const char *options, int (*parse_extra_token)(char *c, void *private),
			void *private);
extern void sraid_destroy_options(struct sraid_options *opt);
extern void sraid_destroy_client(struct sraid_client *client);

extern int sraid_compare_options(struct sraid_options *new_opt,
			 struct sraid_client *client);
extern struct sraid_client *sraid_create_client(struct sraid_options *opt, void *private,
				       u64 supported_features,
				       u64 required_features);
extern int sraid_open_session(struct sraid_client *client);

#endif
