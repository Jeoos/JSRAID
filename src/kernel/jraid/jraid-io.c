/* 
 * jraid-io.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/kernel.h>

#include "jraid-io.h"
#include "jraid-lbd.h"
#include "jraid-pool.h"

blk_qc_t jraid_make_request(struct request_queue *q, struct bio *bi)
{
	const int rw = bio_data_dir(bi);
	unsigned int sectors;
	int cpu;
        struct local_block_device *lbd = q->queuedata;

	sectors = bio_sectors(bi);

        lbd->pers->make_request(lbd, bi);

        /* added for iostat probe */
	cpu = part_stat_lock();
	part_stat_inc(cpu, &lbd->gendisk->part0, ios[rw]);
	part_stat_add(cpu, &lbd->gendisk->part0, sectors[rw], sectors);
	part_stat_unlock();

        bio_endio(bi);
        return BLK_QC_T_NONE;
}
