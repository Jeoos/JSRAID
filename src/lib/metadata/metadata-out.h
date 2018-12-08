/*
 * metadata-out.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "dv.h"
#include "lp.h"

#ifndef __METADATA_OUT_H__
#define __METADATA_OUT_H__

#define SECTOR_SHIFT 9L
#define SECTOR_SIZE ( 1L << SECTOR_SHIFT )

#define FMT_INSTANCE_MDAS		0x00000002U
#define FMT_INSTANCE_AUX_MDAS		0x00000004U
#define FMT_INSTANCE_PRIVATE_MDAS	0x00000008U

struct cmd_context;
struct format_handler;
struct labeller;

struct format_type {
	struct jd_list list;
	struct cmd_context *cmd;
	struct format_handler *ops;
	struct jd_list mda_ops; /* list of permissible mda ops. */
	struct labeller *labeller;
	const char *name;
	const char *orphan_lp_name;
	struct lbd_pool *orphan_lp; /* only one ever exists. */
	void *private;
};

struct lpnameid_list {
	struct jd_list list;
	const char *lp_name;
	const char *lpid;
};

struct dv_list {
	struct jd_list list;
	struct disk_volume *dv;
};

struct dv_create_args {
	uint64_t size;
	uint64_t data_alignment;
	uint64_t data_alignment_offset;
	uint64_t label_sector;
	uint64_t dvmetadatasize;
};

struct dvcreate_params {
	/*
	 * From argc and argv.
	 */
	char **dv_names;
	uint32_t dv_count;
        unsigned is_remove : 1;

        struct dv_create_args dva;

	struct jd_list arg_devices;     /* dvcreate_device, one for each dv_name */
	struct jd_list arg_create;      /* dvcreate_device, used for dvcreate */
	struct jd_list arg_remove;
	struct jd_list arg_fail;        /* dvcreate_device, failed to create */
	struct jd_list arg_process;        /* dvcreate_device, for create process */

        struct jd_list dvs;
};

struct lpcreate_params {
	const char *lp_name;
	uint32_t extent_size;
	size_t max_dv;
	size_t max_lbd;
	uint32_t lpmetadatacopies;
	const char *system_id;
};

struct format_instance {
	unsigned ref_count;	/* refs to this fid from LP and DV structs */

	uint32_t type;
	const struct format_type *fmt;

	/* FIXME: try to use the index only. remove these lists. */
	struct jd_list metadata_areas_in_use;
	struct jd_list metadata_areas_ignored;
	struct jd_hash_table *metadata_areas_index;

	void *private;
};

int is_orphan_lp(const char *lp_name);

struct disk_volume *dv_create(struct cmd_context *cmd,
				  struct device *dev,
				  struct dv_create_args *dva);
int dv_write(struct cmd_context *cmd, struct disk_volume *dv);

void dv_defaults_init(struct dvcreate_params *dp);

int get_lpnameids(struct cmd_context *cmd, struct jd_list *lpnameids,
                int include_internal);

struct lbd_pool *lp_read(struct cmd_context *cmd, const char *lp_name,
                const char *lpid, uint32_t read_flags, uint32_t lockd_state);

struct lbd_pool *_lp_read(struct cmd_context *cmd, const char *lp_name,
			     const char *lpid, uint32_t read_flags);

void add_dvl_to_lps(struct lbd_pool *lp, struct dv_list *dvl);

struct lbd_pool *lp_create(struct cmd_context *cmd, const char *lp_name);

int lp_write(struct lbd_pool *lp);
#endif
