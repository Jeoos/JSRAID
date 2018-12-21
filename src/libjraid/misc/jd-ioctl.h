/*
 * jd-ioctl.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef __JD_IOCTL_H__
#define __JD_IOCTL_H__

#define JD_DIR "jdlbd"

#define JD_CONTROL_NODE "control"
#define JD_NAME_LEN 128
#define JD_UUID_LEN 129

struct jd_ioctl {
	/*
	 * The version number is made up of three parts:
	 * major - no backward or forward compatibility,
	 * minor - only backwards compatible,
	 * patch - both backwards and forwards compatible.
         */

	uint32_t version[3];

	uint32_t data_size;	/* total size of data passed in
				 * including this struct */

	uint32_t data_start;	/* offset to start of data
				 * relative to start of this struct */

	uint32_t flags;		/* in/out */
	uint64_t dev;

	char name[JD_NAME_LEN];	/* device name */
	char uuid[JD_UUID_LEN];	/* unique identifier for
				 * the block device */
};

#endif
