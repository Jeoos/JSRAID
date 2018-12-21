/*
 * libjd-targets.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __LIBJD_TARGETS_H__
#define __LIBJD_TARGETS_H__

struct jd_ioctl;

struct target {
	uint64_t start;
	uint64_t length;
	char *type;
	char *params;

	struct target *next;
};

struct jd_task {
	int type;
	char *dev_name;

	struct target *head, *tail;

	int major;
	int minor;
	uint32_t read_ahead_flags;
	union {
		struct jd_ioctl *v4;
	} jmi;

	char *uuid;
};

#endif
