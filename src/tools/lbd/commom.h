/*
 * commom.h
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

#endif
