/*
 * lbdself.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include "common.h"

#define READ_FOR_UPDATE		0x00100000U

struct lbdcreate_cmdline_params {
	uint64_t size;
	char **dvs;
	uint32_t dv_count;
};

struct processing_params {
	struct lbdcreate_params *lbd_p;
	struct lbdcreate_cmdline_params *lbd_cp;
};

static int _lbdcreate_params(struct cmd_context *cmd,
			    int argc, char **argv,
			    struct lbdcreate_params *lbd_p,
			    struct lbdcreate_cmdline_params *lbd_cp)
{
        return 1;
}

static int _check_pool_parameters(struct cmd_context *cmd,
				  struct lbd_pool *lp,
				  struct lbdcreate_params *lbd_p,
				  struct lbdcreate_cmdline_params *lbd_cp)
{
        return 1;
}

static int _lbdcreate_single(struct cmd_context *cmd, const char *lp_name,
			    struct lbd_pool *lp, struct processing_handle *handle)
{
        return 1;
}

int lbdself(struct cmd_context *cmd, int argc, char **argv)
{
	struct processing_handle *handle = NULL;
	struct processing_params pp;
	int ret;

        printf("in lbdself argc:%d argv:%s\n", argc, argv[2]);

	struct lbdcreate_params lbd_p = {
		.major = -1,
		.minor = -1,
	};
	struct lbdcreate_cmdline_params lbd_cp = { 0 };

	if (!_lbdcreate_params(cmd, argc, argv, &lbd_p, &lbd_cp)) {
                printf("err: failed fo get lbd create params.\n");
		return EINVALID_CMD_LINE;
	}

	if (!_check_pool_parameters(cmd, NULL, &lbd_p, &lbd_cp)) {
                printf("err: failed fo check lbd pool params.\n");
		return EINVALID_CMD_LINE;
	}

	pp.lbd_p = &lbd_p;
	pp.lbd_cp = &lbd_cp;

        if (!(handle = init_processing_handle(cmd, NULL))) {
                printf("err: failed to initialize processing handle.\n");
		return ECMD_FAILED;
	}

	handle->custom_handle = &pp;

	ret = process_each_lp(cmd, 0, NULL, lbd_p.lp_name, NULL, READ_FOR_UPDATE, 0, handle,
			      &_lbdcreate_single);

        return ret;
}
