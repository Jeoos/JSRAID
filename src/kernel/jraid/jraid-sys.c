/* 
 * jraid-sys.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "jraid-sys.h"
#include "jraid-dv.h"
#include "jraid-pool.h"
#include "jraid-lbd.h"

#include <linux/blkdev.h>

struct kobject *kobj_base;
struct local_block_device *lbd;
extern struct jraid_pool *jd_pool;

static ssize_t
add_store(struct kobject *kobj, struct kobj_attribute *attr,
                                const char *page, size_t cnt)
{
        int total_num, jraid_num, ret;
        char filename[MAX_NAME_LEN];
        struct disk_volume *dv;

        dv = (struct disk_volume*)kzalloc(sizeof(*dv), GFP_KERNEL);
        if (!dv)
                return -ENOMEM;

        sscanf(page, "%s %d %d",filename, &total_num, &jraid_num);
        strcpy(dv->filename, filename);

        printk("total_num=%d jraid_num=%d\n", total_num, jraid_num);

        dv->bdev = blkdev_get_by_path(filename, FMODE_WRITE | FMODE_READ | FMODE_EXCL,
                                dv);
        if (IS_ERR_OR_NULL(dv->bdev))
                goto err_bdev;
        dv->sectors = get_capacity(dv->bdev->bd_disk);
        dv->desc_nr = jraid_num;
        printk("dv=%p dv->desc_nr = %u\n", dv, dv->desc_nr);

        list_add(&dv->plist, &jd_pool->dvs);
        
        if (jraid_num == (total_num - 1)) {
                lbd = lbd_alloc();
                if (!lbd) {
                        ret = EINVAL;
                        goto err_alloc;
                }
        }

        return (ssize_t)cnt;
err_alloc:
err_bdev:
        kfree(dv);
        return -ENODEV;
}

static ssize_t
del_store(struct kobject *kobj, struct kobj_attribute *attr,
                                const char *page, size_t cnt)
{
        char filename[MAX_NAME_LEN];
        struct disk_volume *dv; 

        sscanf(page, "%s", filename);

        list_for_each_entry(dv, &jd_pool->dvs, plist) {
                if (!strcmp(dv->filename, filename)){
                        list_del_init(&dv->plist);
                        break;
                }
        }

	blkdev_put(dv->bdev, FMODE_READ | FMODE_WRITE | FMODE_EXCL);
        kfree(dv);

        return (ssize_t)cnt;
}

static ssize_t
mngt_show(struct kobject *kobj, struct kobj_attribute *attr, 
                                        char *page)
{
        ssize_t ret = 0;

        ret += snprintf(page, PAGE_SIZE, 
                "echo filename total_num jraid_num > \
                /sys/jraid/add\n");
        ret += snprintf(page + ret, PAGE_SIZE, 
                "echo filename total_num jraid_num > \
                /sys/jraid/del\n");

        ret += snprintf(page + ret, PAGE_SIZE, 
                "echo /dev/sdb 4 0 > \
                /sys/jraid/add\n");
        return ret;
}

static struct kobj_attribute jraid_define_attr[] = {
        __ATTR(mngt, S_IRUGO, mngt_show, NULL),
        __ATTR(add, S_IWUSR, NULL, add_store),
        __ATTR(del, S_IWUSR, NULL, del_store),
};

static const struct attribute *jraid_define[] = {
        &jraid_define_attr[0].attr,
        &jraid_define_attr[1].attr,
        &jraid_define_attr[2].attr,
        NULL,
};

int jraid_sys_init(void)
{
        int ret;

        kobj_base = kobject_create_and_add("jraid", NULL);
        if (!kobj_base) {
                ret = -ENOMEM;
                goto err_base;
        }
        ret = sysfs_create_files(kobj_base, jraid_define);
        if (ret)
                goto err_base_files;

        return 0;
err_base_files:
        kobject_del(kobj_base);
        kobject_put(kobj_base);
err_base:
        return ret;
}

void jraid_sys_exit(void)
{
        sysfs_remove_files(kobj_base, jraid_define);
        kobject_del(kobj_base);
        kobject_put(kobj_base);
}
