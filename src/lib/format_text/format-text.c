/*
 * format-text.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "format-text.h"
#include "lbdcache.h"
#include "toolcontext.h"
#include "label.h"
#include <malloc.h>
#include <string.h>

struct text_fid_context {
	char *raw_metadata_buf;
	uint32_t raw_metadata_buf_size;
};

struct dir_list {
	struct jd_list list;
	char dir[0];
};

struct raw_list {
	struct jd_list list;
	struct device_area dev_area;
};

static int _text_dv_initialise(const struct format_type *fmt,
			       struct dv_create_args *dva,
			       struct disk_volume *dv)
{
        return 1;
}

static int _text_dv_write(const struct format_type *fmt, struct disk_volume *dv)
{
	struct label *label = NULL;
	struct lbdcache_info *info;
	struct lbdcache_lpinfo *lpinfo = fmt->orphan_lp->lpinfo;

        info = lbdcache_info_from_lpinfo(dv->dev, lpinfo);
        if (!info)
                return 0;

	label = lbdcache_get_label(info);
        /* label write */
	if (!label_write(dv->dev, label)) {
                printf("err: label write.\n");
		return 0;
	}
        return 1;
}

struct cached_lp_fmtdata {
        uint32_t cached_mda_checksum;
        size_t cached_mda_size;
};

static struct lbd_pool *_lp_read_file(struct format_instance *fi,
					  const char *lpname,
					  struct metadata_area *mda,
					  struct cached_lp_fmtdata **lp_fmtdata,
					  unsigned *use_previous_lp __attribute__((unused)))
{
	return NULL; 
}

static int _lp_write_file(struct format_instance *fid __attribute__((unused)),
			  struct lbd_pool *lp, struct metadata_area *mda)
{
        return 1;
}

static struct metadata_area_ops _metadata_text_file_ops = {
	.lp_read = _lp_read_file,
	.lp_write = _lp_write_file
};

static void *_create_text_context(struct text_context *tc)
{
        return NULL;
}

static struct lbd_pool *_lp_read_raw(struct format_instance *fi,
					 const char *lpname,
					 struct metadata_area *mda,
					 struct cached_lp_fmtdata **lp_fmtdata,
					 unsigned *use_previous_lp)
{
	return NULL;
}

static int _lp_write_raw(struct format_instance *fi, struct lbd_pool *lp,
			 struct metadata_area *mda)
{
        return 1;
}

static struct metadata_area_ops _metadata_text_raw_ops = {
	.lp_read = _lp_read_raw,
	.lp_write = _lp_write_raw
};

static int _raw_holds_lpname(struct format_instance *fid,
			     struct device_area *dev_area, const char *lpname)
{
        return 1;
}

static int _create_lp_text_instance(struct format_instance *fid,
                                    const struct format_instance_ctx *fic)
{
	static char path[PATH_MAX];
	uint32_t type = fic->type;
	struct text_fid_context *fidtc;
	struct metadata_area *mda;
	struct dir_list *dl;
	struct raw_list *rl;
	struct jd_list *dir_list, *raw_list;
        struct text_context tc;
	struct mda_context *mdac;
	const char *lp_name, *lp_id;

	if (!(fidtc = malloc(sizeof(*fidtc)))) {
                printf("err: couldn't allocate text_fid_context.\n");
		return 0;
        }

	fid->private = (void *) fidtc;

	if (type & FMT_INSTANCE_PRIVATE_MDAS) {
                /* FIXME: type FMT_INSTANCE_PRIVATE_MDAS */
        } else {
                /* FIXME: FMT_INSTANCE_AUX_MDAS */ 
		lp_name = fic->context.lp_ref.lp_name;
		lp_id = fic->context.lp_ref.lp_id;

		if (!(fid->metadata_areas_index = jd_hash_create(128))) {
                        printf("err: couldn't create metadata index for format "
				  "instance of LP %s.", lp_name);
			return 0;
                }
	        if (type & FMT_INSTANCE_AUX_MDAS) {
			dir_list = &((struct mda_lists *) fid->fmt->private)->dirs;
			jd_list_iterate_items(dl, dir_list) {
				if (snprintf(path, PATH_MAX, "%s/%s", dl->dir, lp_name) < 0) {
                                        printf("err: name too long %s/%s", dl->dir, lp_name);
					return 0;
				}

				if (!(mda = malloc(sizeof(*mda))))
					return 0;
				mda->ops = &_metadata_text_file_ops;
				tc.path_live = path;
				tc.path_edit = tc.desc = NULL;
				mda->metadata_locn = _create_text_context(&tc);
				mda->status = 0;
				fid_add_mda(fid, mda, NULL, 0, 0);
			}

			raw_list = &((struct mda_lists *) fid->fmt->private)->raws;
			jd_list_iterate_items(rl, raw_list) {
				/* FIXME Cache this; rescan below if some missing */
				if (!_raw_holds_lpname(fid, &rl->dev_area, lp_name))
					continue;

				if (!(mda = malloc(sizeof(*mda))))
					return 0;

				if (!(mdac = malloc(sizeof(*mdac))))
					return 0;
				mda->metadata_locn = mdac;
				/* FIXME allow multiple dev_areas inside area */
				memcpy(&mdac->area, &rl->dev_area, sizeof(mdac->area));
				mda->ops = &_metadata_text_raw_ops;
				mda->status = 0;
				/* FIXME MISTAKE? mda->metadata_locn = context; */
				fid_add_mda(fid, mda, NULL, 0, 0);
			}
                }
        }
        return 1;
}

static struct format_instance *_text_create_text_instance(const struct format_type *fmt,
							  const struct format_instance_ctx *fic)
{
	struct format_instance *fid;

	if (!(fid = alloc_fid(fmt, fic)))
		return NULL;

	if (!_create_lp_text_instance(fid, fic)) {
		return NULL;
	}

	return fid;
}


static void _text_destroy(struct format_type *fmt)
{
        if (fmt->orphan_lp)
                free_orphan_lp(fmt->orphan_lp);
        free(fmt);
}

static struct format_handler _text_handler = {
	.dv_initialise = _text_dv_initialise,
	.dv_write = _text_dv_write,
	.create_instance = _text_create_text_instance,
	.destroy = _text_destroy
};

struct format_type *create_text_format(struct cmd_context *cmd)
{
	struct format_type *fmt;
	if (!(fmt = malloc(sizeof(*fmt)))) {
		printf("failed to allocate text format type structure.\n");
		return NULL;
	}
	fmt->cmd = cmd;
	fmt->ops = &_text_handler;
	fmt->name = FMT_TEXT_NAME;
	fmt->orphan_lp_name = ORPHAN_LP_NAME(FMT_TEXT_NAME);

	if (!(fmt->orphan_lp = alloc_lp("text_orphan", cmd, fmt->orphan_lp_name))){
		printf("couldn't create orphan lbd pool.\n");
                goto err_out;
        }

	if (!(fmt->labeller = text_labeller_create(fmt))) {
		printf("couldn't create text label handler.\n");
		goto err_out;
	}

        return fmt;

err_out:
        _text_destroy(fmt);
	return NULL;
}
