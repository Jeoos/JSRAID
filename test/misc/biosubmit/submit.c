/* *  radix_demo.c interface
*
*  Copyright(C)  2018
*  Contact: JeCortex@yahoo.com
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/slab.h>
#include <linux/bio.h>
#include <linux/kernel.h>

#include "blkdev-sys.h"
#include "ioctl_us.h"

#define SECTOR_SHIFT    9
#define PAGE_SECTORS_SHIFT  (PAGE_SHIFT - SECTOR_SHIFT)                                                                          
#define PAGE_SECTORS        (1 << PAGE_SECTORS_SHIFT)

static void io_end_bio(struct bio *bio)
{
	//struct Tblock_device *Tdev = bio->bi_private;
        if (bio->bi_error) {
		printk(KERN_ERR "%s-%d: Not uptodata bio try again !%ld \n",
		        __func__, __LINE__, bio->bi_iter.bi_sector);
                return ;
        }
	//__free_pages(page, 0);
	bio_put(bio);
}

int submit_readwrite(int rw,struct Tblock_device *Tdev, struct page *page, sector_t sector)
{
	struct bio *bio;
	bio = bio_kmalloc(GFP_KERNEL, 1);
	if (!bio) {
		printk(KERN_ERR "%s-%d: Cannot alloc bio\n", __func__, __LINE__);
		return -ENOMEM;
	}
	/* init bio */
	bio->bi_bdev = Tdev->bdev;
	bio->bi_private = Tdev;

	bio->bi_rw |= REQ_FAILFAST_DEV |
		REQ_FAILFAST_TRANSPORT |
		REQ_FAILFAST_DRIVER;

	bio_add_page(bio, page, PAGE_SIZE, 0);
/*
	bio->bi_end_io = io_end_bio;

	submit_bio(rw, bio);
*/
	submit_bio_wait(rw, bio);
        bio_put(bio);

	return 0;
}

static int blk_dev_write(struct Tblock_device *Tdev, struct page *page, unsigned int len, 
                unsigned int off, sector_t sector, uint64_t bi_sector, uint32_t bi_size)
{
        int nr,i;
	unsigned int offset = (sector & (PAGE_SECTORS -1)) << SECTOR_SHIFT;
	unsigned int copy = min_t(unsigned int, len, PAGE_SIZE - offset);
        char sectors;
        void *dst = NULL, *src = NULL;
        struct page *tpage;
        src = page_address(page)+off;
        nr = (len>(PAGE_SIZE-offset))?2:1;

	for (i=0; i<nr; i++) {
                tpage = alloc_page(GFP_KERNEL);
                if (!tpage) {
                        printk(KERN_ERR "%s-%d: Cannot alloc page\n", __func__, __LINE__);
                        return -ENOMEM;
                }
                sectors = copy >> SECTOR_SHIFT;
                src += i*(len-copy);
                dst = page_address(tpage)+offset;
                memcpy(dst, src, copy);

                submit_readwrite(WRITE, Tdev, tpage, sector);

		sector += sectors;
		copy = len-copy;
		offset = 0;
                /* FIXME: async write, wait for io_end_bio */
                __free_pages(tpage, 0);
        }
        return 0;
}

static int blk_dev_read(struct Tblock_device *Tdev, struct page *page, unsigned int len, 
                        unsigned int off, sector_t sector)
{
	int nr, i;
	unsigned int offset = (sector & (PAGE_SECTORS -1)) << SECTOR_SHIFT;
	unsigned int copy = min_t(unsigned int, len, PAGE_SIZE - offset);
	char sectors;
	void *dst = NULL, *src = NULL;
        struct page *tpage;
	dst = page_address(page)+off;
	nr = (len>(PAGE_SIZE-offset))?2:1;

        for(i=0;i<nr;i++){
                tpage = alloc_page(GFP_KERNEL);
                if (!tpage) {
                        printk(KERN_ERR "%s-%d: Cannot alloc page\n", __func__, __LINE__);
                        return -ENOMEM;
                }
                sectors = copy >>SECTOR_SHIFT;
		dst += i*(len-copy);

	        src = page_address(tpage)+offset;
                submit_readwrite(READ, Tdev, tpage, sector);

                /* FIXME: async read, wait for io_end_bio */
	        memcpy(dst, src, copy);
		sector += sectors;
		copy = len-copy;
		offset = 0;
                __free_pages(tpage, 0);
        }
        return 0;
}

blk_qc_t Tdev_make_request(struct request_queue *q, struct bio *bi)
{
        int rw, err;
        struct bio_vec bvec;
        struct bvec_iter iter;
        sector_t sector = bi->bi_iter.bi_sector;
        struct Tblock_device *Tdev = q->queuedata;

        rw = bio_rw(bi);
        //printk("a rw=%d bi->bi_iter.bi_sector=%lu bi->bi_iter.bi_size=%lu\n", 
        //       rw, bi->bi_iter.bi_sector, bi->bi_iter.bi_size);

        bio_for_each_segment(bvec, bi, iter) {
                //printk("bvec.bv_len=%u bvec.bv_offset=%lu\n", bvec.bv_len, 
                //       bvec.bv_offset);
                if (rw == WRITE) {
                        err = blk_dev_write(Tdev, bvec.bv_page, bvec.bv_len,
                                bvec.bv_offset, sector,
                                bi->bi_iter.bi_sector, bi->bi_iter.bi_size);
                } else {
                        err = blk_dev_read(Tdev, bvec.bv_page, bvec.bv_len,
                                bvec.bv_offset, sector);
                }
                if (err) break;
                sector += bvec.bv_len >> SECTOR_SHIFT;
        }
        bio_endio(bi);

        return BLK_QC_T_NONE;
}

static int __init submit_init(void)
{
        blkdev_sys_init();
        return 0;
}

static void __exit submit_exit(void)
{
        blkdev_sys_exit();
        return;
}

module_init(submit_init);
module_exit(submit_exit);

MODULE_LICENSE("GPL");
