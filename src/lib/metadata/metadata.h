/*
 * metadata.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __METADATA_H__
#define __METADATA_H__

#include "metadata-out.h"

struct cached_lp_fmtdata;

struct metadata_area {
	struct jd_list list;
	struct metadata_area_ops *ops;
	void *metadata_locn;
	uint32_t status;
};

struct metadata_area_ops {
	struct jd_list list;
	struct lbd_pool *(*lp_read) (struct format_instance * fi,
					 const char *lp_name,
					 struct metadata_area * mda,
					 struct cached_lp_fmtdata **lp_fmtdata,
					 unsigned *use_previous_lp);

	int (*lp_write) (struct format_instance *fid, struct lbd_pool *lp,
			 struct metadata_area *mda);
};

struct format_instance_ctx {
	uint32_t type;
	union {
		const char *dv_id;
		struct {
			const char *lp_name;
			const char *lp_id;
		} lp_ref;
		void *private;
	} context;
};

struct format_handler {

	int (*dv_read) (const struct format_type * fmt, const char *dv_name,
			struct disk_volume* dv, int scan_label_only);

	int (*dv_initialise) (const struct format_type * fmt,
			      struct dv_create_args *dva,
			      struct disk_volume* dv);

	int (*dv_write) (const struct format_type * fmt,
			 struct disk_volume* dv);

	struct format_instance *(*create_instance) (const struct format_type *fmt,
						const struct format_instance_ctx *fic);

	void (*destroy) (struct format_type * fmt);
};

struct format_instance *alloc_fid(const struct format_type *fmt,
				  const struct format_instance_ctx *fic);

int fid_add_mda(struct format_instance *fid, struct metadata_area *mda,
		 const char *key, size_t key_len, const unsigned sub_key);
#endif
