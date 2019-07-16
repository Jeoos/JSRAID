/* 
 * blkdev-sys.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "blkdev-sys.h"
#include "ioctl_us.h"

#include <linux/blkdev.h>

struct kobject *kobj_base;

#define SIMP_BLKDEV_DEVICEMAJOR COMPAQ_SMART2_MAJOR

LIST_HEAD(Tdev_list);
extern blk_qc_t Tdev_make_request(struct request_queue *q, struct bio *bi);

static int Tdev_open(struct block_device *bdev, fmode_t mode)
{
        return 0;
}

static void Tdev_release(struct gendisk *disk, fmode_t mode)
{
        return;
}

static inline bool Tdev_ioctl_valid(unsigned int cmd)
{
        switch (cmd) {
        case TEST_01:
        case TEST_02:
                return true;
        default:
                return false;
        }
}

static int Tdev_ioctl(struct block_device *bdev, fmode_t mode,                    
                        unsigned int cmd, unsigned long arg)                    
{ 

        if(!Tdev_ioctl_valid(cmd))
                return -ENOTTY;

        switch (cmd) {
        case TEST_01:
                goto out_deal;
        case TEST_02:
                break;
        default:
                ;
        }

out_deal:
        printk("do something ...\n");

        return 0;
}


static const struct block_device_operations Tdev_blkdev_fops = {
        .owner          = THIS_MODULE,
        .open           = Tdev_open,
        .release        = Tdev_release,
        .ioctl          = Tdev_ioctl, 
};

static ssize_t
add_store(struct kobject *kobj, struct kobj_attribute *attr,
                                const char *page, size_t cnt)
{
        char filename[MAX_NAME_LEN];
        struct Tblock_device *Tdev;

        Tdev = (struct Tblock_device*)kzalloc(sizeof(*Tdev), GFP_KERNEL);
        if (!Tdev)
                return -ENOMEM;

        sscanf(page, "%s",filename);
        strcpy(Tdev->filename, filename);

        Tdev->bdev = blkdev_get_by_path(filename, FMODE_WRITE | FMODE_READ | FMODE_EXCL,
                                Tdev);
        if (IS_ERR_OR_NULL(Tdev->bdev))
                goto err_bdev;

        Tdev->queue = blk_alloc_queue(GFP_KERNEL);
        if (!Tdev->queue) {
                goto err_queue;
        }

        strcpy(Tdev->Tdev_name, "Tdev0");
        Tdev->queue->queuedata = Tdev;

        blk_queue_make_request(Tdev->queue, Tdev_make_request);

        Tdev->gendisk = alloc_disk(1);
        if (!Tdev->gendisk) {
                goto err_disk;
        }

        strcpy(Tdev->gendisk->disk_name, Tdev->Tdev_name);
        Tdev->gendisk->major = SIMP_BLKDEV_DEVICEMAJOR ;
        Tdev->gendisk->first_minor = 0;
        Tdev->gendisk->fops = &Tdev_blkdev_fops;
        Tdev->gendisk->queue = Tdev->queue;
       
        Tdev->sectors = get_capacity(Tdev->bdev->bd_disk);

        set_capacity(Tdev->gendisk, Tdev->sectors);
        blk_queue_flush(Tdev->gendisk->queue, REQ_FLUSH | REQ_FUA);
        
        add_disk(Tdev->gendisk);
        list_add_tail(&Tdev->list, &Tdev_list);

        return (ssize_t)cnt;

err_disk:
        blk_cleanup_queue(Tdev->queue);
err_queue:
	blkdev_put(Tdev->bdev, FMODE_READ | FMODE_WRITE | FMODE_EXCL);
err_bdev:
        kfree(Tdev);
        return -ENODEV;
}

static ssize_t
del_store(struct kobject *kobj, struct kobj_attribute *attr,
                                const char *page, size_t cnt)
{
        struct Tblock_device *Tdev;
        char filename[MAX_NAME_LEN];

        sscanf(page, "%s", filename);

        list_for_each_entry(Tdev, &Tdev_list, list) {
                if (!strcmp(Tdev->filename, filename)) {
                        list_del_init(&Tdev->list);
                        break;
                }
        }

        del_gendisk(Tdev->gendisk);
        put_disk(Tdev->gendisk);
        blk_cleanup_queue(Tdev->queue);
	blkdev_put(Tdev->bdev, FMODE_READ | FMODE_WRITE | FMODE_EXCL);
        kfree(Tdev);

        return (ssize_t)cnt;
}

static ssize_t
mngt_show(struct kobject *kobj, struct kobj_attribute *attr, 
                                        char *page)
{
        ssize_t ret = 0;

        ret += snprintf(page, PAGE_SIZE, 
                "echo filename > /sys/Tblkdev/add\n");
        ret += snprintf(page + ret, PAGE_SIZE, 
                "echo filename > /sys/Tblkdev/del\n");

        ret += snprintf(page + ret, PAGE_SIZE, 
                "echo /dev/sdb > /sys/Tblkdev/add\n");
        return ret;
}

static struct kobj_attribute blkdev_define_attr[] = {
        __ATTR(mngt, S_IRUGO, mngt_show, NULL),
        __ATTR(add, S_IWUSR, NULL, add_store),
        __ATTR(del, S_IWUSR, NULL, del_store),
};

static const struct attribute *blkdev_define[] = {
        &blkdev_define_attr[0].attr,
        &blkdev_define_attr[1].attr,
        &blkdev_define_attr[2].attr,
        NULL,
};

int blkdev_sys_init(void)
{
        int ret;

        kobj_base = kobject_create_and_add("Tblkdev", NULL);
        if (!kobj_base) {
                ret = -ENOMEM;
                goto err_base;
        }
        ret = sysfs_create_files(kobj_base, blkdev_define);
        if (ret)
                goto err_base_files;

        return 0;
err_base_files:
        kobject_del(kobj_base);
        kobject_put(kobj_base);
err_base:
        return ret;
}

void blkdev_sys_exit(void)
{
        sysfs_remove_files(kobj_base, blkdev_define);
        kobject_del(kobj_base);
        kobject_put(kobj_base);
}
