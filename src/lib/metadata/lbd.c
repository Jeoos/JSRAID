/*
 * lbd.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "metadata.h"
#include "toolcontext.h"

int lbd_active_change(struct cmd_context *cmd, struct logical_block_device *lbd,
		     enum activation_change activate, int needs_exclusive)
{
        return 1;
}
