/*
 * dv.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __DV_H__
#define __DV_H__

#include "libjraid.h"

struct device;

struct disk_volume {
        struct device *dev;
};

#endif