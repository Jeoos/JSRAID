/*
 * lbd.c for the kernel software
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/workqueue.h>
#include <linux/backing-dev.h>

#include "libsraid.h"

static bool single_major = false;                                               
module_param(single_major, bool, S_IRUGO);
MODULE_PARM_DESC(single_major, "Use a single major number for all lbd devices (default: false)");

#define	SECTOR_SHIFT	9
#define	SECTOR_SIZE	(1ULL << SECTOR_SHIFT)

#define LBD_MINORS_PER_MAJOR		256
#define LBD_SINGLE_MAJOR_PART_SHIFT	4

#define LBD_DRV_NAME "rbd"
#define DEV_NAME_LEN  32
static int lbd_major;

static LIST_HEAD(lbd_dev_list);    /* devices */
static DEFINE_SPINLOCK(lbd_dev_list_lock);

static LIST_HEAD(lbd_client_list);		/* clients */
static DEFINE_SPINLOCK(lbd_client_list_lock);

static struct kmem_cache	*lbd_img_request_cache;
static struct kmem_cache	*lbd_obj_request_cache;

static struct device_type lbd_device_type;
static struct device lbd_root_dev;

static struct workqueue_struct *lbd_wq;

/*
 * Instance of the client. multiple devices may share one lbd client.
 */
struct lbd_client {
        struct sraid_client     *client; 
        struct kref             kref;
        struct list_head        node;
};

struct lbd_obj_request {

};

struct lbd_img_request {

};

/*
 * Lbd image specification.
 */
struct lbd_spec {
        u64              pool_id; 
        const char      *pool_name;

        const char      *image_id;
        const char      *image_name;

        struct kref     kref;
};

struct lbd_options {
        int     queue_depth;
        bool    read_only;
};

struct lbd_mapping {
	u64                     size;
	u64                     features;
	bool			read_only;
};

struct lbd_image_header {
	char *object_prefix;
	__u8 obj_order;
	__u8 crypt_type;
	__u8 comp_type;
	u64 stripe_unit;
	u64 stripe_count;
	u64 features;		/* Might be changeable someday? */

	u64 image_size;
};

struct lbd_device {
        int                     dev_id;         /* blkdev unique id */ 

        int                     major;          /* blkdev assigned major */
        int                     minor;
        struct gendisk          *disk;          /* blkdev's gendisk */
	u32			image_format;	/* either 1 or 2 */

        struct lbd_client       *lbd_client;
        char                    name[DEV_NAME_LEN]; /* blkdev name, e.g lbd0 */

	spinlock_t		lock;		/* queue, flags, open_count */

	struct lbd_image_header	header;
        unsigned long           flags;
        struct lbd_spec         *spec;
        struct lbd_options      *opts;

	char			*header_name;

        struct lbd_device       *parent;
	struct lbd_mapping	mapping;

	struct blk_mq_tag_set	tag_set;

        struct list_head        node;

        /* sysfs related */
        struct device           dev;
        unsigned long           open_count;
};

enum lbd_dev_flags {
        LBD_DEV_FLAG_EXISTS,
        LBD_DEV_FLAG_REMOVING,
};

static ssize_t lbd_add(struct bus_type *bus, const char *buf,
                        size_t count);
static ssize_t lbd_remove(struct bus_type *bus, const char *buf,
                        size_t count);
static ssize_t lbd_add_single_major(struct bus_type *bus, const char *buf,
                        size_t count);
static ssize_t lbd_remove_single_major(struct bus_type *bus, const char *buf,
			size_t count);

static struct lbd_spec *lbd_spec_alloc(void);
static void lbd_spec_put(struct lbd_spec *spec);
static void lbd_dev_destroy(struct lbd_device *lbd_dev);

static struct lbd_device *dev_to_lbd_dev(struct device *dev);

static BUS_ATTR(add, S_IWUSR, NULL, lbd_add);
static BUS_ATTR(remove, S_IWUSR, NULL, lbd_remove);
static BUS_ATTR(add_single_major, S_IWUSR, NULL, lbd_add_single_major);
static BUS_ATTR(remove_single_major, S_IWUSR, NULL, lbd_remove_single_major);

static struct attribute *lbd_bus_attrs[] = {
        &bus_attr_add.attr,
        &bus_attr_remove.attr,
	&bus_attr_add_single_major.attr,
	&bus_attr_remove_single_major.attr,
        NULL,
};

static umode_t lbd_bus_is_visible(struct kobject *kobj,                         
                struct attribute *attr, int index)            
{                                                                               
	if (!single_major &&
	    (attr == &bus_attr_add_single_major.attr ||
	     attr == &bus_attr_remove_single_major.attr))
                return 0;

        return attr->mode;
}

static const struct attribute_group lbd_bus_group = {
        .attrs = lbd_bus_attrs,
        .is_visible = lbd_bus_is_visible,
};
__ATTRIBUTE_GROUPS(lbd_bus);

static struct bus_type lbd_bus_type = {
        .name           = "lbd",
        .bus_groups     = lbd_bus_groups,
};

static inline size_t next_token(const char **buf)
{
        const char *spaces = " \f\n\r\t\v";

        /* find start of token */
        *buf += strspn(*buf, spaces);

        /* return token length */
	return strcspn(*buf, spaces);
}

static inline char *dup_token(const char **buf, size_t *lenp)
{
	char *dup;
	size_t len;

	len = next_token(buf);
	dup = kmemdup(*buf, len + 1, GFP_KERNEL);
	if (!dup)
		return NULL;
	*(dup + len) = '\0';
	*buf += len;

	if (lenp)
		*lenp = len;

	return dup;
}

#define LBD_QUEUE_DEPTH_DEFAULT	BLKDEV_MAX_RQ
#define LBD_READ_ONLY_DEFAULT	false

static int parse_lbd_opts_token(char *c, void *private)
{
        return 0;
}

static int lbd_add_parse_args(const char *buf, 
                                struct sraid_options **sraid_opts,
                                struct lbd_options **opts,
                                struct lbd_spec **lbd_spec)
{
	size_t len;
        int rt;
	const char *options;

	struct lbd_spec *spec = NULL;
	struct lbd_options *lbd_opts = NULL;
	struct sraid_options *sopts;

	len = next_token(&buf);
	if (!len) {
		printk("no options provided");
		return -EINVAL;
	}
        options = buf;
	buf += len;

        rt =-EINVAL;

	spec = lbd_spec_alloc();
	if (!spec)
		goto out_mem;

	spec->pool_name = dup_token(&buf, NULL);
	if (!spec->pool_name)
		goto out_mem;
	if (!*spec->pool_name) {
		printk("no pool name provided");
		goto out_err;
        }

	spec->image_name = dup_token(&buf, NULL);
	if (!spec->image_name)
		goto out_mem;
	if (!*spec->image_name) {
		printk("no image name provided");
		goto out_err;
        }

	lbd_opts = kzalloc(sizeof (*lbd_opts), GFP_KERNEL);
	if (!lbd_opts)
		goto out_mem;

	lbd_opts->read_only = LBD_READ_ONLY_DEFAULT;
	lbd_opts->queue_depth = LBD_QUEUE_DEPTH_DEFAULT;

	sopts = sraid_parse_options(options, parse_lbd_opts_token, lbd_opts);
	if (IS_ERR(sopts)) {
		rt = PTR_ERR(sopts);
		goto out_err;
	}
	kfree(options);

	*sraid_opts = sopts;
	*opts = lbd_opts;
	*lbd_spec = spec;

        return -1; 

out_mem:
	rt = -ENOMEM;
out_err:
	kfree(lbd_opts);
	lbd_spec_put(spec);
	kfree(options);
        return rt;
}

static struct lbd_client *__lbd_get_client(struct lbd_client *lbdc)
{
	kref_get(&lbdc->kref);

	return lbdc;
}

static struct lbd_client *lbd_client_find(struct sraid_options *sraid_opts)
{
        struct lbd_client *client_node;
	bool found = false;

	if (sraid_opts->flags & SRAID_OPT_NOSHARE)
		return NULL;

	spin_lock(&lbd_client_list_lock);
	list_for_each_entry(client_node, &lbd_client_list, node) {
		if (!sraid_compare_options(sraid_opts, client_node->client)) {
			__lbd_get_client(client_node);

			found = true;
			break;
		}
	}
	spin_unlock(&lbd_client_list_lock);

	return found ? client_node : NULL;
}

static int lbd_open(struct block_device *bdev, fmode_t mode)
{
	struct lbd_device *lbd_dev = bdev->bd_disk->private_data;
	bool removing = false;

	if ((mode & FMODE_WRITE) && lbd_dev->mapping.read_only)
		return -EROFS;

	spin_lock_irq(&lbd_dev->lock);
	if (test_bit(LBD_DEV_FLAG_REMOVING, &lbd_dev->flags))
		removing = true;
	else
		lbd_dev->open_count++;
	spin_unlock_irq(&lbd_dev->lock);
	if (removing)
		return -ENOENT;

	(void) get_device(&lbd_dev->dev);

	return 0;
}

static void lbd_release(struct gendisk *disk, fmode_t mode)
{
	struct lbd_device *lbd_dev = disk->private_data;
	unsigned long open_count_before;

	spin_lock_irq(&lbd_dev->lock);
	open_count_before = lbd_dev->open_count--;
	spin_unlock_irq(&lbd_dev->lock);

        /* BUG_ON(open_count_before > 0) */
	put_device(&lbd_dev->dev);
}

static int lbd_ioctl(struct block_device *bdev, fmode_t mode,
			unsigned int cmd, unsigned long arg)
{
        return 0;
}

static const struct block_device_operations lbd_bd_ops = {
	.owner			= THIS_MODULE,
	.open			= lbd_open,
	.release		= lbd_release,
	.ioctl			= lbd_ioctl,
};

static struct lbd_client *lbd_client_create(struct sraid_options *sraid_opts)
{
	struct lbd_client *lbdc;
	int rt = -ENOMEM;

	printk("%s:\n", __func__);
	lbdc = kmalloc(sizeof(struct lbd_client), GFP_KERNEL);
	if (!lbdc)
		goto out_opt;

	kref_init(&lbdc->kref);
	INIT_LIST_HEAD(&lbdc->node);

	lbdc->client = sraid_create_client(sraid_opts, lbdc, 0, 0);
	if (IS_ERR(lbdc->client))
		goto out_lbdc;
	sraid_opts = NULL;

	rt = sraid_open_session(lbdc->client);
	if (rt < 0)
		goto out_client;

	spin_lock(&lbd_client_list_lock);
	list_add_tail(&lbdc->node, &lbd_client_list);
	spin_unlock(&lbd_client_list_lock);

	printk("%s: lbdc %p\n", __func__, lbdc);

	return lbdc;
out_client:
	sraid_destroy_client(lbdc->client);
out_lbdc:
	kfree(lbdc);
out_opt:
	if (sraid_opts)
		sraid_destroy_options(sraid_opts);
	printk("%s: error %d\n", __func__, rt);

	return ERR_PTR(rt);
}

static struct lbd_client *lbd_get_client(struct sraid_options *sraid_opts)
{
        struct lbd_client *lbdc;

        /* fix me: lock */
        lbdc = lbd_client_find(sraid_opts);
        if (lbdc)
                sraid_destroy_options(sraid_opts);
        else
                lbdc = lbd_client_create(sraid_opts);
        /* unlock */

        return lbdc;
}

static void lbd_client_release(struct kref *kref)
{

}

static void lbd_put_client(struct lbd_client *lbdc)
{
        if (lbdc)
                kref_put(&lbdc->kref, lbd_client_release);
}

static int lbd_add_get_pool_id(struct lbd_client *lbdc, const char *pool_name)
{
        return 0;
}

static struct lbd_device *lbd_dev_create(struct lbd_client *lbdc,
                                        struct lbd_spec *spec,
                                        struct lbd_options *opts)
{
	struct lbd_device *lbd_dev;

	lbd_dev = kzalloc(sizeof (*lbd_dev), GFP_KERNEL);
	if (!lbd_dev)
		return NULL;

	spin_lock_init(&lbd_dev->lock);
	lbd_dev->flags = 0;
	INIT_LIST_HEAD(&lbd_dev->node);

	lbd_dev->dev.bus = &lbd_bus_type;
	lbd_dev->dev.type = &lbd_device_type;
	lbd_dev->dev.parent = &lbd_root_dev;
	device_initialize(&lbd_dev->dev);

	lbd_dev->lbd_client = lbdc;
	lbd_dev->spec = spec;
	lbd_dev->opts = opts;

	if (lbd_dev->opts)
		__module_get(THIS_MODULE);

	return lbd_dev;
}

static void lbd_dev_unprobe(struct lbd_device *lbd_dev)
{

}

static void lbd_dev_image_release(struct lbd_device *lbd_dev)
{
        /* unprobe lbd device */
        lbd_dev_unprobe(lbd_dev);

	kfree(lbd_dev->header_name);
	lbd_dev->header_name = NULL;
	lbd_dev->image_format = 0;
	kfree(lbd_dev->spec->image_id);
	lbd_dev->spec->image_id = NULL;

	lbd_dev_destroy(lbd_dev);
}

static int lbd_dev_image_id(struct lbd_device *lbd_dev)
{
        return 0;
}

static int lbd_dev_header_name(struct lbd_device *lbd_dev)
{
        return 0;
}

static int lbd_dev_image_probe(struct lbd_device *lbd_dev, int depth)
{
        int rt;

        /* get the id from the image id object */
        rt = lbd_dev_image_id(lbd_dev);
        if (rt)
                return rt;

	/* record the header object name for this lbd image */
        rt = lbd_dev_header_name(lbd_dev);
        if (rt)
                goto err_out_format;

        /* ... */

        return 0;

err_out_format:
        return rt;
}

static void lbd_dev_destroy(struct lbd_device *lbd_dev)
{
        if (lbd_dev)
                put_device(&lbd_dev->dev);
}

static int lbd_dev_id_get(struct lbd_device *lbd_dev)
{
        return 0;
}

static void lbd_dev_id_put(struct lbd_device *lbd_dev)
{

}

static int lbd_queue_rq(struct blk_mq_hw_ctx *hctx,
		const struct blk_mq_queue_data *bd)
{
	struct request *rq = bd->rq;
	struct work_struct *work = blk_mq_rq_to_pdu(rq);

	queue_work(lbd_wq, work);
	return BLK_MQ_RQ_QUEUE_OK;
}

static void lbd_queue_workfn(struct work_struct *work)
{
        /* handle request field, blah balh ...*/

}

static int lbd_init_request(void *data, struct request *rq,
		unsigned int hctx_idx, unsigned int request_idx,
		unsigned int numa_node)
{
	struct work_struct *work = blk_mq_rq_to_pdu(rq);

	INIT_WORK(work, lbd_queue_workfn);
	return 0;
}


static struct blk_mq_ops lbd_mq_ops = {
	.queue_rq	= lbd_queue_rq,
	.map_queue	= blk_mq_map_queue,
	.init_request	= lbd_init_request,
};

static u64 lbd_obj_bytes(struct lbd_image_header *header)
{
	return 1 << header->obj_order;
}

static int lbd_init_disk(struct lbd_device *lbd_dev)
{
	struct gendisk *disk;
	struct request_queue *q;
	u64 segment_size;
	int err;

	/* create gendisk info */
	disk = alloc_disk(single_major ?
			  (1 << LBD_SINGLE_MAJOR_PART_SHIFT) :
			  LBD_MINORS_PER_MAJOR);
	if (!disk)
		return -ENOMEM;

	snprintf(disk->disk_name, sizeof(disk->disk_name), LBD_DRV_NAME "%d",
		 lbd_dev->dev_id);
	disk->major = lbd_dev->major;
	disk->first_minor = lbd_dev->minor;
	if (single_major)
		disk->flags |= GENHD_FL_EXT_DEVT;
	disk->fops = &lbd_bd_ops;
	disk->private_data = lbd_dev;

	memset(&lbd_dev->tag_set, 0, sizeof(lbd_dev->tag_set));
	lbd_dev->tag_set.ops = &lbd_mq_ops;
	lbd_dev->tag_set.queue_depth = lbd_dev->opts->queue_depth;
	lbd_dev->tag_set.numa_node = NUMA_NO_NODE;
	lbd_dev->tag_set.flags = BLK_MQ_F_SHOULD_MERGE | BLK_MQ_F_SG_MERGE;
	lbd_dev->tag_set.nr_hw_queues = 1;
	lbd_dev->tag_set.cmd_size = sizeof(struct work_struct);

	err = blk_mq_alloc_tag_set(&lbd_dev->tag_set);
	if (err)
		goto out_disk;

	q = blk_mq_init_queue(&lbd_dev->tag_set);
	if (IS_ERR(q)) {
		err = PTR_ERR(q);
		goto out_tag_set;
	}

	queue_flag_set_unlocked(QUEUE_FLAG_NONROT, q);
	/* QUEUE_FLAG_ADD_RANDOM is off by default for blk-mq */

	/* set io sizes to object size */
	segment_size = lbd_obj_bytes(&lbd_dev->header);
	blk_queue_max_hw_sectors(q, segment_size / SECTOR_SIZE);
	q->limits.max_sectors = queue_max_hw_sectors(q);
	blk_queue_max_segments(q, segment_size / SECTOR_SIZE);
	blk_queue_max_segment_size(q, segment_size);
	blk_queue_io_min(q, segment_size);
	blk_queue_io_opt(q, segment_size);

	/* enable the discard support */
	queue_flag_set_unlocked(QUEUE_FLAG_DISCARD, q);
	q->limits.discard_granularity = segment_size;
	q->limits.discard_alignment = segment_size;
	blk_queue_max_discard_sectors(q, segment_size / SECTOR_SIZE);
	q->limits.discard_zeroes_data = 1;

	q->backing_dev_info.capabilities |= BDI_CAP_STABLE_WRITES;

	disk->queue = q;

	q->queuedata = lbd_dev;

	lbd_dev->disk = disk;

	return 0;
out_tag_set:
	blk_mq_free_tag_set(&lbd_dev->tag_set);
out_disk:
	put_disk(disk);
	return err;
}

static void lbd_free_disk(struct lbd_device *lbd_dev)
{
	struct gendisk *disk = lbd_dev->disk;

	if (!disk)
		return;

	lbd_dev->disk = NULL;
	if (disk->flags & GENHD_FL_UP) {
		del_gendisk(disk);
		if (disk->queue)
			blk_cleanup_queue(disk->queue);
		blk_mq_free_tag_set(&lbd_dev->tag_set);
	}
	put_disk(disk);
}

static int lbd_dev_mapping_set(struct lbd_device *lbd_dev)
{
        u64 size = 0;
        u64 features = 0;

	lbd_dev->mapping.size = size;
	lbd_dev->mapping.features = features;
        return 0;
} 

static void lbd_dev_mapping_clear(struct lbd_device *lbd_dev)
{
	lbd_dev->mapping.size = 0;
	lbd_dev->mapping.features = 0;
}

static int lbd_dev_device_setup(struct lbd_device *lbd_dev)
{
        int ret;

        /* get an id and fill in device name */
	ret = lbd_dev_id_get(lbd_dev);
	if (ret)
		return ret;

	sprintf(lbd_dev->name, "%s%d", LBD_DRV_NAME, lbd_dev->dev_id);

	/* record our major and minor device numbers. */

	if (!single_major) {
		ret = register_blkdev(0, lbd_dev->name);
		if (ret < 0)
			goto err_out_id;

		lbd_dev->major = ret;
		lbd_dev->minor = 0;
	} else {
		lbd_dev->major = lbd_major;
		//lbd_dev->minor = lbd_dev_id_to_minor(lbd_dev->dev_id);
	}

	/* set up the blkdev mapping. */

	ret = lbd_init_disk(lbd_dev);
	if (ret)
		goto err_out_blkdev;

	ret = lbd_dev_mapping_set(lbd_dev);
	if (ret)
		goto err_out_disk;

	set_capacity(lbd_dev->disk, lbd_dev->mapping.size / SECTOR_SIZE);
	set_disk_ro(lbd_dev->disk, lbd_dev->mapping.read_only);

	dev_set_name(&lbd_dev->dev, "%d", lbd_dev->dev_id);
	ret = device_add(&lbd_dev->dev);
	if (ret)
		goto err_out_mapping;

	/* everything's ready.  announce the disk to the world. */

	set_bit(LBD_DEV_FLAG_EXISTS, &lbd_dev->flags);
	add_disk(lbd_dev->disk);

	pr_info("%s: added with size 0x%llx\n", lbd_dev->disk->disk_name,
		(unsigned long long) lbd_dev->mapping.size);

	return ret;

err_out_mapping:
	lbd_dev_mapping_clear(lbd_dev);
err_out_disk:
	lbd_free_disk(lbd_dev);
err_out_blkdev:
	if (!single_major)
		unregister_blkdev(lbd_dev->major, lbd_dev->name);
err_out_id:
	lbd_dev_id_put(lbd_dev);
	return ret;
}

static void lbd_dev_header_unwatch_sync(struct lbd_device *lbd_dev)
{

}

static struct lbd_spec *lbd_spec_alloc(void)
{
	struct lbd_spec *spec;

	spec = kzalloc(sizeof (*spec), GFP_KERNEL);
	if (!spec)
		return NULL;

	spec->pool_id = SRAID_NOPOOL;
	kref_init(&spec->kref);

	return spec;
}

static ssize_t lbd_size_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct lbd_device *lbd_dev = dev_to_lbd_dev(dev);

	return sprintf(buf, "%llu\n",
		(unsigned long long)lbd_dev->mapping.size);
}

static DEVICE_ATTR(size, S_IRUGO, lbd_size_show, NULL);

static struct attribute *lbd_attrs[] = {
	&dev_attr_size.attr,
	NULL
};

static struct attribute_group lbd_attr_group = {
	.attrs = lbd_attrs,
};

static const struct attribute_group *lbd_attr_groups[] = {
	&lbd_attr_group,
	NULL
};

static struct lbd_device *dev_to_lbd_dev(struct device *dev)
{
	return container_of(dev, struct lbd_device, dev);
}

static void lbd_dev_release(struct device *dev)
{
	struct lbd_device *lbd_dev = dev_to_lbd_dev(dev);
	bool need_put = !!lbd_dev->opts;

	lbd_put_client(lbd_dev->lbd_client);
	lbd_spec_put(lbd_dev->spec);
	kfree(lbd_dev->opts);
	kfree(lbd_dev);

	if (need_put)
		module_put(THIS_MODULE);
}

static struct device_type lbd_device_type = {
	.name		= "lbd",
	.groups		= lbd_attr_groups,
	.release	= lbd_dev_release,
};

static void lbd_spec_free(struct kref *kref)
{
	struct lbd_spec *spec = container_of(kref, struct lbd_spec, kref);

	kfree(spec->pool_name);
	kfree(spec->image_id);
	kfree(spec->image_name);
	kfree(spec);
}

static void lbd_spec_put(struct lbd_spec *spec)
{
        if (spec)
                kref_put(&spec->kref, lbd_spec_free);
}

static ssize_t do_lbd_add(struct bus_type *bus,
                const char *buf,
                size_t count)
{
        struct lbd_device *lbd_dev = NULL;
        struct sraid_options *sraid_opts = NULL;
        struct lbd_options *lbd_opts = NULL;
        struct lbd_spec *spec = NULL;
        struct lbd_client *lbdc;
        int rc;

        if(!try_module_get(THIS_MODULE))
                return -ENODEV;

        /* parse add cmd */
        rc = lbd_add_parse_args(buf, &sraid_opts, &lbd_opts, &spec);
        if(rc < 0)
                goto out;
        /* get a sraid client */
        lbdc = lbd_get_client(sraid_opts);
        if (IS_ERR(lbdc)) {
                rc = PTR_ERR(lbdc);
                goto err_out_args;
        }

        /* pick the pool */
        rc = lbd_add_get_pool_id(lbdc, spec->pool_name);
        if (rc < 0) {
                if (rc == -ENOENT)
                pr_info("pool %s does not exist\n", spec->pool_name);
                goto err_out_client;

        }
        spec->pool_id = (u64)rc;

        /* the sraid file layout needs to fit pool id in 32 bits */
        if (spec->pool_id > (u64)U32_MAX) {
                printk("pool id too large (%llu > %u)",
                         (unsigned long long)spec->pool_id, U32_MAX);
                rc = -EIO;
                goto err_out_client;
        }
        
        /* create lbd device */
        lbd_dev = lbd_dev_create(lbdc, spec, lbd_opts);
        if (!lbd_dev) {
                rc = -ENOMEM;
                goto err_out_client;
        }
        lbdc = NULL;            /* lbd_dev now owns this */
        spec = NULL;            /* lbd_dev now owns this */
        lbd_opts = NULL;        /* lbd_dev now owns this */
        
        /* probe whether the image is exist */
        rc = lbd_dev_image_probe(lbd_dev, 0);
        if (rc < 0)
                goto err_out_lbd_dev;

        /* time to setup lbd, running as a standardize block device */
        rc = lbd_dev_device_setup(lbd_dev);
        if (rc) {
                lbd_dev_header_unwatch_sync(lbd_dev);
                lbd_dev_image_release(lbd_dev);
                goto out;
        }

        rc = count;
out:
        module_put(THIS_MODULE);
        return rc;

err_out_lbd_dev:
        lbd_dev_destroy(lbd_dev);
err_out_client:
        lbd_put_client(lbdc);
err_out_args:
        lbd_spec_put(spec);
        kfree(lbd_opts);
        goto out;
}

static ssize_t lbd_add(struct bus_type *bus,
                const char *buf,
                size_t count)
{
        if (single_major)
                return -EINVAL;

        return do_lbd_add(bus, buf, count);
}

static ssize_t lbd_add_single_major(struct bus_type *bus,
				    const char *buf,
				    size_t count)
{
	return do_lbd_add(bus, buf, count);
}

static void lbd_dev_device_release(struct lbd_device *lbd_dev)
{
	lbd_free_disk(lbd_dev);
	clear_bit(LBD_DEV_FLAG_EXISTS, &lbd_dev->flags);
	device_del(&lbd_dev->dev);
	lbd_dev_mapping_clear(lbd_dev);
	if (!single_major)
		unregister_blkdev(lbd_dev->major, lbd_dev->name);
	lbd_dev_id_put(lbd_dev);
}

static ssize_t do_lbd_remove(struct bus_type *bus,
                const char *buf,
                size_t count)
{
        int rt, dev_id;
	unsigned long ul;
	bool already = false;
        struct lbd_device *lbd_dev = NULL;
	struct list_head *tmp;

	rt = kstrtoul(buf, 10, &ul);
	if (rt)
		return rt;

	/* convert to int, and check */
	dev_id = (int)ul;
	if (dev_id != ul)
		return -EINVAL;

	rt = -ENOENT;
	list_for_each(tmp, &lbd_dev_list) {
		lbd_dev = list_entry(tmp, struct lbd_device, node);
		if (lbd_dev->dev_id == dev_id) {
			rt = 0;
			break;
		}
	}

	if (!rt) {
		spin_lock_irq(&lbd_dev->lock);
		if (lbd_dev->open_count)
			rt = -EBUSY;
		else
			already = test_and_set_bit(LBD_DEV_FLAG_REMOVING,
							&lbd_dev->flags);
		spin_unlock_irq(&lbd_dev->lock);
	}
	spin_unlock(&lbd_dev_list_lock);
	if (rt < 0 || already)
                return rt;

        lbd_dev_header_unwatch_sync(lbd_dev);

	lbd_dev_device_release(lbd_dev);
	lbd_dev_image_release(lbd_dev);

        return count;
}

static ssize_t lbd_remove(struct bus_type *bus,
                const char *buf,
                size_t count)
{                                                                               
        if (single_major)
                return -EINVAL;

        return do_lbd_remove(bus, buf, count);
}

static ssize_t lbd_remove_single_major(struct bus_type *bus,
				       const char *buf,
				       size_t count)
{
	return do_lbd_remove(bus, buf, count);
}

static void lbd_root_dev_release(struct device *dev)
{

}

static struct device lbd_root_dev = {
        .init_name =    "lbd",
        .release =      lbd_root_dev_release,
};    

static int lbd_sysfs_init(void)
{
        int rt;

        rt = device_register(&lbd_root_dev);
        if (rt < 0)
                return rt;

        rt = bus_register(&lbd_bus_type);
        if (rt < 0)
                device_unregister(&lbd_root_dev);

        return rt;
}

static void lbd_sysfs_cleanup(void)
{                                                                               
        bus_unregister(&lbd_bus_type);
        device_unregister(&lbd_root_dev);
}

static int lbd_slab_init(void)
{
	lbd_img_request_cache = kmem_cache_create("lbd_img_request",
					sizeof (struct lbd_img_request),
					__alignof__(struct lbd_img_request),
					0, NULL);
	if (!lbd_img_request_cache)
		return -ENOMEM;

	lbd_obj_request_cache = kmem_cache_create("lbd_obj_request",
					sizeof (struct lbd_obj_request),
					__alignof__(struct lbd_obj_request),
					0, NULL);
	if (!lbd_obj_request_cache)
		goto err_out;

        return 0;

err_out:
	kmem_cache_destroy(lbd_obj_request_cache);
	lbd_obj_request_cache = NULL;

	kmem_cache_destroy(lbd_img_request_cache);
	lbd_img_request_cache = NULL;

        return -ENOMEM;
}

static void lbd_slab_exit(void)
{
	kmem_cache_destroy(lbd_obj_request_cache);
	lbd_obj_request_cache = NULL;

	kmem_cache_destroy(lbd_img_request_cache);
	lbd_img_request_cache = NULL;
}

static int __init lbd_init(void)
{
        int rc = 0;
        printk("lbd init, blah blah ...\n");

	rc = lbd_slab_init();
	if (rc)
		return rc;

	lbd_wq = alloc_workqueue(LBD_DRV_NAME, WQ_MEM_RECLAIM, 0);
	if (!lbd_wq) {
		rc = -ENOMEM;
		goto err_out_slab;
	}

        if (single_major) {
                lbd_major = register_blkdev(0, LBD_DRV_NAME);
                if (lbd_major < 0) {
                        rc = lbd_major;
                        goto err_out_wq;
                } 
        }

        rc = lbd_sysfs_init();
        if (rc)
                goto err_out_blkdev;

	if (single_major)
		printk("loaded (major %d)\n", lbd_major);
	else
		printk("loaded\n");
        
        return 0;

err_out_blkdev:
	if (single_major)
                unregister_blkdev(lbd_major, LBD_DRV_NAME);
err_out_wq:
	destroy_workqueue(lbd_wq);
err_out_slab:
        lbd_slab_exit();
        return rc;
}

static void __exit lbd_exit(void)
{
        printk("lbd exit, blah blah ...\n");
        lbd_sysfs_cleanup();
        if (single_major)
                unregister_blkdev(lbd_major, LBD_DRV_NAME);
	destroy_workqueue(lbd_wq);
	lbd_slab_exit();
}

module_init(lbd_init);
module_exit(lbd_exit);

MODULE_LICENSE("GPL");
