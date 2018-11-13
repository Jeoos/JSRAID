/*
 * common.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

 #ifndef __COMMOM_H__
 #define __COMMOM_H__

#include <stdint.h> 
#include <string.h>
#include <sys/types.h> 
#include "errors.h"
#include "lbd-toollib.h"
#include "toolcontext.h"
#include "dev-cache.h"

#define ENABLE_ALL_DEVS 0x00000008

enum {
#define arg(a, b, c, d, e, f, g) a ,
#include "args.h"
#undef arg
};

enum {
#define cmd(a, b) a ,
#include "cmds.h"
#undef cmd
};

struct arg_values {
	unsigned count;
	char *value;
	int32_t i_value;
	uint32_t ui_value;
	int64_t i64_value;
	uint64_t ui64_value;
};

extern int lbdself(struct cmd_context *cmd, int argc, char **argv);
extern int lbdpool(struct cmd_context *cmd, int argc, char **argv);
extern int dvcreate(struct cmd_context *cmd, int argc, char **argv);
extern int dvremove(struct cmd_context *cmd, int argc, char **argv);

#endif
