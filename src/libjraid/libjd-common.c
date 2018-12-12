/*
 * libjd-common.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "libjd-common.h"
#include "jd-ioctl.h"

#include <sys/param.h>

#define DEV_DIR "/dev/"

static char _jd_dir[PATH_MAX] = DEV_DIR JD_DIR;

const char *jd_dir(void)
{
	return _jd_dir;
}
