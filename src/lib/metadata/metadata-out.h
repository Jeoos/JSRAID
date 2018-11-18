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

struct cmd_context;
struct format_handler;
struct format_type {
	struct jd_list list;
	struct cmd_context *cmd;
	struct format_handler *ops;
	struct jd_list mda_ops; /* list of permissible mda ops. */
	const char *name;
	const char *orphan_lp_name;
	struct lbd_pool *orphan_lp; /* only one ever exists. */
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
};

struct format_instance {

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

#endif
