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
#include <malloc.h>
#include <string.h>


#define IOCTL_MAPPER 
#define T_NAME "lp0-lbdol0" 
#define T_UUID "LBD-0kK2P2wY0fw3awp8mOP0yL5SRRVSVHGPZAjggD613KUwRu5nPI5GrC2oXQOAFchS"

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
        jdi = malloc(sizeof(*jdi));
        if (!jdi) {
                printf("err: failed to alloc jd_ioctl.\n");
                return NULL;
        }
        
        if (jdt->do_remove)
                goto do_rm;

#ifdef IOCTL_MAPPER
        command = 3241737475;
        jdi->version[0]=4; 
        jdi->version[1]=0;
        jdi->version[2]=0;
        jdi->data_size=16384;
        jdi->data_start=312;
        jdi->flags=516;
        jdi->dev=0;
        strcpy(jdi->name, T_NAME);
        strcpy(jdi->uuid, T_UUID);
#endif

	ioctl(_control_fd, command, jdi);
        printf("_control_fd=%d\n", _control_fd);

        return jdi;
do_rm:

#ifdef IOCTL_MAPPER
        command = 3241737476;
        jdi->version[0]=4; 
        jdi->version[1]=0;
        jdi->version[2]=0;
        jdi->data_size=16384;
        jdi->data_start=312;
        jdi->flags=524;
        jdi->dev=0;
        strcpy(jdi->name, T_NAME);
        //strcpy(jdi->uuid, T_UUID);
#endif
	ioctl(_control_fd, command, jdi);

        return NULL;
} 

int jd_task_run(struct jd_task *jdt)
{
	struct jd_ioctl *jdi;

        if (!_open_control()) {
                printf("err: failed to open control.\n");
                return 0;
        }

	if (!(jdi = _do_jd_ioctl(jdt, 0, 0, 0, NULL)) && !jdt->do_remove) {

                /* FIXME: for ioctl retry */

                printf("err: failed to ioctl.\n");
                return 0;
        }
        return 1;
}
