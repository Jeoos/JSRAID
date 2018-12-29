/*
 * test-ioctl.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __TEST_IOCTL_H__
#define __TEST_IOCTL_H__

struct jd_ioctl;

#define T_NAME "vg0-lvol0" 
#define T_UUID "LVM-0kK2P2wY0fw3awp8mOP0yL5SRRVSVHGPZAjggD613KUwRu5nPI5GrC2oXQOAFchS"

#define VALUE_CREATE      3241737475
#define VALUE_TABLE_LOAD  3241737481
#define VALUE_SUSPEND     3241737478
#define VALUE_REMOVE      3241737476

typedef enum {
        DEV_CREATE,
        DEV_TABLE_LOAD,
        DEV_SUSPEND,
        DEV_REMOVE
} ioctl_t;

int test_ioctl(int fd, unsigned command, struct jd_ioctl *jdi, ioctl_t do_ioctl);

#endif
