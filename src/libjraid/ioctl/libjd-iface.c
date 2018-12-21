/*
 * libjd-iface.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "libjraid.h"
#include "jd-ioctl.h"
#include "libjd-targets.h"

#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>

static int _control_fd = -1;

static int _open_and_assign_control_fd(const char *control)
{
	if ((_control_fd = open(control, O_RDWR)) < 0) {
		printf("open %s failed.\n", control);
		return 0;
	}

	return 1;
}

static int _open_control(void)
{
	char control[PATH_MAX];

	if (_control_fd != -1)
		return 1;

	if (snprintf(control, sizeof(control), "%s/%s", jd_dir(), JD_CONTROL_NODE) < 0)
		goto bad;

	if (!_open_and_assign_control_fd(control))
		goto bad;

        return 1;
bad:
        return 0;
}

static struct jd_ioctl *_do_jd_ioctl(struct jd_task *jdt, unsigned command,
				     unsigned buffer_repeat_count,
				     unsigned retry_repeat_count,
				     int *retryable)
{
	struct jd_ioctl *jdi = NULL;

        /* FIXME: a jd_ioctl needed, alloc and init it */

	ioctl(_control_fd, command, jdi);

        return jdi;
}

int jd_task_run(struct jd_task *jdt)
{
	struct jd_ioctl *jdi;

        if (!_open_control()) {
                printf("err: failed to open control.\n");
                return 0;
        }

	if (!(jdi = _do_jd_ioctl(jdt, 0, 0, 0, NULL))) {

                /* FIXME: for ioctl retry */

                printf("err: failed to ioctl.\n");
                return 0;
        }
        return 1;
}
