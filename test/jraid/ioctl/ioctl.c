/* 
 * ioctl_test.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ioctl_us.h"

/* simple test, find out the device MAJOR manual. */
#define BLKDEV_DEVICEMAJOR 72

int main ()
{
        int fd;

        /* find out the dev name manual. */
        fd = open("/dev/lbd0", O_RDONLY);

        ioctl(fd, TEST_0, NULL);
        ioctl(fd, TEST_1, NULL);
        close(fd);
        return 0;

}
