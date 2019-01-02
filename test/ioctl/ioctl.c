/*
 * ioctl.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "jd-ioctl.h"
#include "test-ioctl.h"

#include <string.h>
#include <sys/ioctl.h>

/*
 * Yep! it's kinds of weird test for devmapper ioctl.
 *
 * Execution steps:
 * $ pvcreate /dev/sdb
 * $ vgcreate vg0 /dev/sdb
 * $ lbd lbdself --create lbd[x]
 * $ lbd lbdself --remove lbd[x]
 *
 * lsblk to see the test result, if success,
 * dd /dev/vg0/vg0-lvol0 will be fine and just
 * like the normal lvm. 
 *
 * BTW, when DEV_TABLE_LOAD invoke
 * we need a patch for kernel dm-mod.ko.
 *
 * struct dm_target_spec filing in:
 * spec->sector_start = 0;
 * spec->length = 41943040;
 * spec->status = 0;
 * spec->next = 56;
 * strcpy(spec->target_type, "linear");
 * 
 * and more, linear_ctr function parameters should be:
 * argc = 2
 * argv[0] = "8:16"
 * argv[1] = "2048"
 */
static int test_jdi_setting(struct jd_ioctl *jdi, ioctl_t do_ioctl)
{
        switch (do_ioctl) {
        case DEV_CREATE:
                jdi->version[0]=4; 
                jdi->version[1]=0;
                jdi->version[2]=0;
                jdi->data_size=16384;
                jdi->data_start=312;
                jdi->target_count=0;
                jdi->open_count=0;
                jdi->flags=516;
                jdi->event_nr=0;
                jdi->padding=0;
                jdi->dev=0;
                strcpy(jdi->name, T_NAME);
                strcpy(jdi->uuid, T_UUID);
                return 1;
        case DEV_TABLE_LOAD:
                jdi->version[0]=4; 
                jdi->version[1]=0;
                jdi->version[2]=0;
                jdi->data_size=16384;
                jdi->data_start=312;
                jdi->target_count=1;
                jdi->open_count=0;
                jdi->flags=524;
                jdi->event_nr=0;
                jdi->padding=0;
                jdi->dev=64768;
                strcpy(jdi->name, "");
                strcpy(jdi->uuid, "");
                return 1;
        case DEV_SUSPEND:
                jdi->version[0]=4; 
                jdi->version[1]=0;
                jdi->version[2]=0;
                jdi->data_size=16384;
                jdi->data_start=312;
                jdi->target_count=0;
                jdi->open_count=0;
                jdi->flags=524;
                jdi->event_nr=23114754;
                jdi->padding=0;
                jdi->dev=64768;
                strcpy(jdi->name, "");
                strcpy(jdi->uuid, "");
                return 1;
        case DEV_REMOVE:
                jdi->version[0]=4; 
                jdi->version[1]=0;
                jdi->version[2]=0;
                jdi->data_size=16384;
                jdi->data_start=312;
                jdi->target_count=0;
                jdi->open_count=0;
                jdi->flags=524;
                jdi->event_nr=6312906;
                jdi->padding=0;
                jdi->dev=0;
                strcpy(jdi->name, T_NAME);
                return 1;
        default:
                return 0;
        }
        return 0;
}

int test_ioctl(int fd, unsigned command, struct jd_ioctl *jdi, ioctl_t do_ioctl)
{
        test_jdi_setting(jdi, do_ioctl);

	ioctl(fd, command, jdi);
        return 1;
}
