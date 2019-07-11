/* 
 * cp_list.c
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

 #include <linux/list.h>
 #include <linux/init.h>
 #include <linux/module.h>
 #include <linux/slab.h>

struct list_cp_from {
        int value;
        struct list_head items;      
};

struct list_cp_to {
        int value;
        struct list_head items;      
};

struct demo_item {
        int value;
        struct list_head flist;      
        struct list_head tlist;      
};

struct list_cp_from *from;
struct list_cp_to *to;

static int __init list_handle_init(void)
{
        int i;
        struct demo_item *item;

        from = kmalloc(sizeof(struct list_cp_from), GFP_KERNEL);
        if (!from) {
                goto err_alloc_from;
        }

        to = kmalloc(sizeof(struct list_cp_to), GFP_KERNEL);
        if (!from) {
                goto err_alloc_to;
        }
        INIT_LIST_HEAD(&from->items);
        INIT_LIST_HEAD(&to->items);
        
        /* add item to from */
        for (i=0; i<3; i++) {
                item = kmalloc(sizeof(struct demo_item), GFP_KERNEL);
                item->value = i;
                /* FIXME: */
                if (!from) {
                        return -EINVAL;
                }
                list_add(&item->flist, &from->items);
        }

        /* cp item from from to to */
        list_for_each_entry(item, &from->items, flist) {
                printk("item=%p &item->flist=%p item->value = %d\n", item, 
                       &item->flist, item->value);
                /* bug ... */
                //list_add(&item->list, &to->items);
                list_add(&item->tlist, &to->items);
        }

        return 0;

err_alloc_to:
        kfree(from);
err_alloc_from:
        return -EINVAL;
}

static void __exit list_handle_exit(void)
{
        struct demo_item *item;
        list_for_each_entry(item, &to->items, tlist) {
                kfree(item);
        }
        kfree(from);
        kfree(to);
        return;
}

module_init(list_handle_init);
module_exit(list_handle_exit);
MODULE_LICENSE("GPL");
