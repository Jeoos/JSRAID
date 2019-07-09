/* 
 * jraid-io.h
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __JRAID_IO_H__
#define __JRAID_IO_H__

#include <linux/bio.h>

blk_qc_t jraid_make_request(struct request_queue *q, struct bio *bi);

#endif
