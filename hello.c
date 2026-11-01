// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/moduleparam.h>
#include <linux/ktime.h>
#include <linux/slab.h>
#include <linux/list.h>

MODULE_AUTHOR("Ігор Чорний");
MODULE_DESCRIPTION("Hello world module with list and time tracking");
MODULE_LICENSE("Dual BSD/GPL");

static uint hello_count = 1;
module_param(hello_count, uint, 0444);
MODULE_PARM_DESC(hello_count, "Number of times to print Hello, world!");

struct hello_event {
	struct list_head list;
	ktime_t timestamp;
};

static LIST_HEAD(hello_list);

static int __init hello_init(void)
{
	uint i;
	struct hello_event *event;

	if (hello_count > 10) {
		pr_err("Parameter hello_count is too large (> 10)\n");
		return -EINVAL;
	}

	if (hello_count == 0 || (hello_count >= 5 && hello_count <= 10))
		pr_warn("Warning: hello_count is %u\n", hello_count);

	for (i = 0; i < hello_count; i++) {
		event = kmalloc(sizeof(*event), GFP_KERNEL);
		if (!event) {
			struct hello_event *cursor, *tmp;

			pr_err("Memory allocation failed\n");
			list_for_each_entry_safe(cursor, tmp, &hello_list, list) {
				list_del(&cursor->list);
				kfree(cursor);
			}
			return -ENOMEM;
		}

		event->timestamp = ktime_get();
		list_add_tail(&event->list, &hello_list);
		pr_info("Hello, world!\n");
	}

	return 0;
}

static void __exit hello_exit(void)
{
	struct hello_event *cursor, *tmp;

	list_for_each_entry_safe(cursor, tmp, &hello_list, list) {
		pr_info("Time: %lld ns\n", ktime_to_ns(cursor->timestamp));
		list_del(&cursor->list);
		kfree(cursor);
	}
}

module_init(hello_init);
module_exit(hello_exit);
