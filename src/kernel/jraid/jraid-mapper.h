/* 
 * jraid-mapper.h
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _LINUX_DEVICE_MAPPER_H
#define _LINUX_DEVICE_MAPPER_H

#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/math64.h>
#include <linux/ratelimit.h>

struct jd_dev;
struct jd_target;
struct jd_table;
struct mapped_device;
struct bio_vec;

typedef enum { STATUSTYPE_INFO, STATUSTYPE_TABLE } status_type_t;

union map_info {
	void *ptr;
};

/*
 * In the constructor the target parameter will already have the
 * table, type, begin and len fields filled in.
 */
typedef int (*jd_ctr_fn) (struct jd_target *target,
			  unsigned int argc, char **argv);

/*
 * The destructor doesn't need to free the jd_target, just
 * anything hidden ti->private.
 */
typedef void (*jd_dtr_fn) (struct jd_target *ti);

/*
 * The map function must return:
 * < 0: error
 * = 0: The target will handle the io by resubmitting it later
 * = 1: simple remap complete
 * = 2: The target wants to push back the io
 */
typedef int (*jd_map_fn) (struct jd_target *ti, struct bio *bio);
typedef int (*jd_map_request_fn) (struct jd_target *ti, struct request *clone,
				  union map_info *map_context);
typedef int (*jd_clone_and_map_request_fn) (struct jd_target *ti,
					    struct request *rq,
					    union map_info *map_context,
					    struct request **clone);
typedef void (*jd_release_clone_request_fn) (struct request *clone);

/*
 * Returns:
 * < 0 : error (currently ignored)
 * 0   : ended successfully
 * 1   : for some reason the io has still not completed (eg,
 *       multipath target might want to requeue a failed io).
 * 2   : The target wants to push back the io
 */
typedef int (*jd_endio_fn) (struct jd_target *ti,
			    struct bio *bio, int error);
typedef int (*jd_request_endio_fn) (struct jd_target *ti,
				    struct request *clone, int error,
				    union map_info *map_context);

typedef void (*jd_presuspend_fn) (struct jd_target *ti);
typedef void (*jd_presuspend_undo_fn) (struct jd_target *ti);
typedef void (*jd_postsuspend_fn) (struct jd_target *ti);
typedef int (*jd_preresume_fn) (struct jd_target *ti);
typedef void (*jd_resume_fn) (struct jd_target *ti);

typedef void (*jd_status_fn) (struct jd_target *ti, status_type_t status_type,
			      unsigned status_flags, char *result, unsigned maxlen);

typedef int (*jd_message_fn) (struct jd_target *ti, unsigned argc, char **argv);

typedef int (*jd_prepare_ioctl_fn) (struct jd_target *ti,
			    struct block_device **bdev, fmode_t *mode);

/*
 * These iteration functions are typically used to check (and combine)
 * properties of underlying devices.
 * E.g. Does at least one underlying device support flush?
 *      Does any underlying device not support WRITE_SAME?
 *
 * The callout function is called once for each contiguous section of
 * an underlying device.  State can be maintained in *data.
 * Return non-zero to stop iterating through any further devices.
 */
typedef int (*iterate_devices_callout_fn) (struct jd_target *ti,
					   struct jd_dev *dev,
					   sector_t start, sector_t len,
					   void *data);

/*
 * This function must iterate through each section of device used by the
 * target until it encounters a non-zero return code, which it then returns.
 * Returns zero if no callout returned non-zero.
 */
typedef int (*jd_iterate_devices_fn) (struct jd_target *ti,
				      iterate_devices_callout_fn fn,
				      void *data);

typedef void (*jd_io_hints_fn) (struct jd_target *ti,
				struct queue_limits *limits);

/*
 * Returns:
 *    0: The target can handle the next I/O immediately.
 *    1: The target can't handle the next I/O immediately.
 */
typedef int (*jd_busy_fn) (struct jd_target *ti);

void jd_error(const char *message);

struct jd_dev {
	struct block_device *bdev;
	fmode_t mode;
	char name[16];
};

/*
 * Constructors should call these functions to ensure destination devices
 * are opened/closed correctly.
 */
int jd_get_device(struct jd_target *ti, const char *path, fmode_t mode,
		  struct jd_dev **result);
void jd_put_device(struct jd_target *ti, struct jd_dev *d);

/*
 * Information about a target type
 */

struct target_type {
	uint64_t features;
	const char *name;
	struct module *module;
	unsigned version[3];
	jd_ctr_fn ctr;
	jd_dtr_fn dtr;
	jd_map_fn map;
	jd_map_request_fn map_rq;
	jd_clone_and_map_request_fn clone_and_map_rq;
	jd_release_clone_request_fn release_clone_rq;
	jd_endio_fn end_io;
	jd_request_endio_fn rq_end_io;
	jd_presuspend_fn presuspend;
	jd_presuspend_undo_fn presuspend_undo;
	jd_postsuspend_fn postsuspend;
	jd_preresume_fn preresume;
	jd_resume_fn resume;
	jd_status_fn status;
	jd_message_fn message;
	jd_prepare_ioctl_fn prepare_ioctl;
	jd_busy_fn busy;
	jd_iterate_devices_fn iterate_devices;
	jd_io_hints_fn io_hints;

	/* For internal device-mapper use. */
	struct list_head list;
};

/*
 * Target features
 */

/*
 * Any table that contains an instance of this target must have only one.
 */
#define JD_TARGET_SINGLETON		0x00000001
#define jd_target_needs_singleton(type)	((type)->features & JD_TARGET_SINGLETON)

/*
 * Indicates that a target does not support read-only devices.
 */
#define JD_TARGET_ALWAYS_WRITEABLE	0x00000002
#define jd_target_always_writeable(type) \
		((type)->features & JD_TARGET_ALWAYS_WRITEABLE)

/*
 * Any device that contains a table with an instance of this target may never
 * have tables containing any different target type.
 */
#define JD_TARGET_IMMUTABLE		0x00000004
#define jd_target_is_immutable(type)	((type)->features & JD_TARGET_IMMUTABLE)

/*
 * Some targets need to be sent the same WRITE bio severals times so
 * that they can send copies of it to different devices.  This function
 * examines any supplied bio and returns the number of copies of it the
 * target requires.
 */
typedef unsigned (*jd_num_write_bios_fn) (struct jd_target *ti, struct bio *bio);

struct jd_target {
	struct jd_table *table;
	struct target_type *type;

	/* target limits */
	sector_t begin;
	sector_t len;

	/* If non-zero, maximum size of I/O submitted to a target. */
	uint32_t max_io_len;

	/*
	 * A number of zero-length barrier bios that will be submitted
	 * to the target for the purpose of flushing cache.
	 *
	 * The bio number can be accessed with jd_bio_get_target_bio_nr.
	 * It is a responsibility of the target driver to remap these bios
	 * to the real underlying devices.
	 */
	unsigned num_flush_bios;

	/*
	 * The number of discard bios that will be submitted to the target.
	 * The bio number can be accessed with jd_bio_get_target_bio_nr.
	 */
	unsigned num_discard_bios;

	/*
	 * The number of WRITE SAME bios that will be submitted to the target.
	 * The bio number can be accessed with jd_bio_get_target_bio_nr.
	 */
	unsigned num_write_same_bios;

	/*
	 * The minimum number of extra bytes allocated in each bio for the
	 * target to use.  jd_per_bio_data returns the data location.
	 */
	unsigned per_bio_data_size;

	/*
	 * If defined, this function is called to find out how many
	 * duplicate bios should be sent to the target when writing
	 * data.
	 */
	jd_num_write_bios_fn num_write_bios;

	/* target specific data */
	void *private;

	/* Used to provide an error string from the ctr */
	char *error;

	/*
	 * Set if this target needs to receive flushes regardless of
	 * whether or not its underlying devices have support.
	 */
	bool flush_supported:1;

	/*
	 * Set if this target needs to receive discards regardless of
	 * whether or not its underlying devices have support.
	 */
	bool discards_supported:1;

	/*
	 * Set if the target required discard bios to be split
	 * on max_io_len boundary.
	 */
	bool split_discard_bios:1;

	/*
	 * Set if this target does not return zeroes on discarded blocks.
	 */
	bool discard_zeroes_data_unsupported:1;
};

/* Each target can link one of these into the table */
struct jd_target_callbacks {
	struct list_head list;
	int (*congested_fn) (struct jd_target_callbacks *, int);
};

/*
 * For bio-based jd.
 * One of these is allocated for each bio.
 * This structure shouldn't be touched directly by target drivers.
 * It is here so that we can inline jd_per_bio_data and
 * jd_bio_from_per_bio_data
 */
struct jd_target_io {
	struct jd_io *io;
	struct jd_target *ti;
	unsigned target_bio_nr;
	unsigned *len_ptr;
	struct bio clone;
};

static inline void *jd_per_bio_data(struct bio *bio, size_t data_size)
{
	return (char *)bio - offsetof(struct jd_target_io, clone) - data_size;
}

static inline struct bio *jd_bio_from_per_bio_data(void *data, size_t data_size)
{
	return (struct bio *)((char *)data + data_size + offsetof(struct jd_target_io, clone));
}

static inline unsigned jd_bio_get_target_bio_nr(const struct bio *bio)
{
	return container_of(bio, struct jd_target_io, clone)->target_bio_nr;
}

int jd_register_target(struct target_type *t);
void jd_unregister_target(struct target_type *t);

/*
 * Target argument parsing.
 */
struct jd_arg_set {
	unsigned argc;
	char **argv;
};

/*
 * The minimum and maximum value of a numeric argument, together with
 * the error message to use if the number is found to be outside that range.
 */
struct jd_arg {
	unsigned min;
	unsigned max;
	char *error;
};

/*
 * Validate the next argument, either returning it as *value or, if invalid,
 * returning -EINVAL and setting *error.
 */
int jd_read_arg(struct jd_arg *arg, struct jd_arg_set *arg_set,
		unsigned *value, char **error);

/*
 * Process the next argument as the start of a group containing between
 * arg->min and arg->max further arguments. Either return the size as
 * *num_args or, if invalid, return -EINVAL and set *error.
 */
int jd_read_arg_group(struct jd_arg *arg, struct jd_arg_set *arg_set,
		      unsigned *num_args, char **error);

/*
 * Return the current argument and shift to the next.
 */
const char *jd_shift_arg(struct jd_arg_set *as);

/*
 * Move through num_args arguments.
 */
void jd_consume_args(struct jd_arg_set *as, unsigned num_args);

/*-----------------------------------------------------------------
 * Functions for creating and manipulating mapped devices.
 * Drop the reference with jd_put when you finish with the object.
 *---------------------------------------------------------------*/

/*
 * JD_ANY_MINOR chooses the next available minor number.
 */
#define JD_ANY_MINOR (-1)
int jd_create(int minor, struct mapped_device **md);

/*
 * Reference counting for md.
 */
struct mapped_device *jd_get_md(dev_t dev);
void jd_get(struct mapped_device *md);
int jd_hold(struct mapped_device *md);
void jd_put(struct mapped_device *md);

/*
 * An arbitrary pointer may be stored alongside a mapped device.
 */
void jd_set_mdptr(struct mapped_device *md, void *ptr);
void *jd_get_mdptr(struct mapped_device *md);

/*
 * A device can still be used while suspended, but I/O is deferred.
 */
int jd_suspend(struct mapped_device *md, unsigned suspend_flags);
int jd_resume(struct mapped_device *md);

/*
 * Event functions.
 */
uint32_t jd_get_event_nr(struct mapped_device *md);
int jd_wait_event(struct mapped_device *md, int event_nr);
uint32_t jd_next_uevent_seq(struct mapped_device *md);
void jd_uevent_add(struct mapped_device *md, struct list_head *elist);

/*
 * Info functions.
 */
const char *jd_device_name(struct mapped_device *md);
int jd_copy_name_and_uuid(struct mapped_device *md, char *name, char *uuid);
struct gendisk *jd_disk(struct mapped_device *md);
int jd_suspended(struct jd_target *ti);
int jd_noflush_suspending(struct jd_target *ti);
void jd_accept_partial_bio(struct bio *bio, unsigned n_sectors);
union map_info *jd_get_rq_mapinfo(struct request *rq);

struct queue_limits *jd_get_queue_limits(struct mapped_device *md);

/*
 * Geometry functions.
 */
int jd_get_geometry(struct mapped_device *md, struct hd_geometry *geo);
int jd_set_geometry(struct mapped_device *md, struct hd_geometry *geo);

/*-----------------------------------------------------------------
 * Functions for manipulating device-mapper tables.
 *---------------------------------------------------------------*/

/*
 * First create an empty table.
 */
int jd_table_create(struct jd_table **result, fmode_t mode,
		    unsigned num_targets, struct mapped_device *md);

/*
 * Then call this once for each target.
 */
int jd_table_add_target(struct jd_table *t, const char *type,
			sector_t start, sector_t len, char *params);

/*
 * Target_ctr should call this if it needs to add any callbacks.
 */
void jd_table_add_target_callbacks(struct jd_table *t, struct jd_target_callbacks *cb);

/*
 * Finally call this to make the table ready for use.
 */
int jd_table_complete(struct jd_table *t);

/*
 * Target may require that it is never sent I/O larger than len.
 */
int __must_check jd_set_target_max_io_len(struct jd_target *ti, sector_t len);

/*
 * Table reference counting.
 */
struct jd_table *jd_get_live_table(struct mapped_device *md, int *srcu_idx);
void jd_put_live_table(struct mapped_device *md, int srcu_idx);
void jd_sync_table(struct mapped_device *md);

/*
 * Queries
 */
sector_t jd_table_get_size(struct jd_table *t);
unsigned int jd_table_get_num_targets(struct jd_table *t);
fmode_t jd_table_get_mode(struct jd_table *t);
struct mapped_device *jd_table_get_md(struct jd_table *t);

/*
 * Trigger an event.
 */
void jd_table_event(struct jd_table *t);

/*
 * Run the queue for request-based targets.
 */
void jd_table_run_md_queue_async(struct jd_table *t);

/*
 * The device must be suspended before calling this method.
 * Returns the previous table, which the caller must destroy.
 */
struct jd_table *jd_swap_table(struct mapped_device *md,
			       struct jd_table *t);

/*
 * A wrapper around vmalloc.
 */
void *jd_vcalloc(unsigned long nmemb, unsigned long elem_size);

/*-----------------------------------------------------------------
 * Macros.
 *---------------------------------------------------------------*/
#define JD_NAME "jraid-mapper"

#ifdef CONFIG_PRINTK

extern struct ratelimit_state jd_ratelimit_state;

#define jd_ratelimit()	__ratelimit(&jd_ratelimit_state)
#else
#define jd_ratelimit()	0
#endif

#define JDCRIT(f, arg...) \
	printk(KERN_CRIT JD_NAME ": " JD_MSG_PREFIX ": " f "\n", ## arg)

#define JDERR(f, arg...) \
	printk(KERN_ERR JD_NAME ": " JD_MSG_PREFIX ": " f "\n", ## arg)
#define JDERR_LIMIT(f, arg...) \
	do { \
		if (jd_ratelimit())	\
			printk(KERN_ERR JD_NAME ": " JD_MSG_PREFIX ": " \
			       f "\n", ## arg); \
	} while (0)

#define JDWARN(f, arg...) \
	printk(KERN_WARNING JD_NAME ": " JD_MSG_PREFIX ": " f "\n", ## arg)
#define JDWARN_LIMIT(f, arg...) \
	do { \
		if (jd_ratelimit())	\
			printk(KERN_WARNING JD_NAME ": " JD_MSG_PREFIX ": " \
			       f "\n", ## arg); \
	} while (0)

#define JDINFO(f, arg...) \
	printk(KERN_INFO JD_NAME ": " JD_MSG_PREFIX ": " f "\n", ## arg)
#define JDINFO_LIMIT(f, arg...) \
	do { \
		if (jd_ratelimit())	\
			printk(KERN_INFO JD_NAME ": " JD_MSG_PREFIX ": " f \
			       "\n", ## arg); \
	} while (0)

#ifdef CONFIG_JD_DEBUG
#  define JDDEBUG(f, arg...) \
	printk(KERN_DEBUG JD_NAME ": " JD_MSG_PREFIX " DEBUG: " f "\n", ## arg)
#  define JDDEBUG_LIMIT(f, arg...) \
	do { \
		if (jd_ratelimit())	\
			printk(KERN_DEBUG JD_NAME ": " JD_MSG_PREFIX ": " f \
			       "\n", ## arg); \
	} while (0)
#else
#  define JDDEBUG(f, arg...) do {} while (0)
#  define JDDEBUG_LIMIT(f, arg...) do {} while (0)
#endif

#define JDEMIT(x...) sz += ((sz >= maxlen) ? \
			  0 : scnprintf(result + sz, maxlen - sz, x))

#define SECTOR_SHIFT 9

/*
 * Definitions of return values from target end_io function.
 */
#define JD_ENDIO_INCOMPLETE	1
#define JD_ENDIO_REQUEUE	2

/*
 * Definitions of return values from target map function.
 */
#define JD_MAPIO_SUBMITTED	0
#define JD_MAPIO_REMAPPED	1
#define JD_MAPIO_REQUEUE	JD_ENDIO_REQUEUE

#define jd_sector_div64(x, y)( \
{ \
	u64 _res; \
	(x) = div64_u64_rem(x, y, &_res); \
	_res; \
} \
)

/*
 * Ceiling(n / sz)
 */
#define jd_div_up(n, sz) (((n) + (sz) - 1) / (sz))

#define jd_sector_div_up(n, sz) ( \
{ \
	sector_t _r = ((n) + (sz) - 1); \
	sector_div(_r, (sz)); \
	_r; \
} \
)

/*
 * ceiling(n / size) * size
 */
#define jd_round_up(n, sz) (jd_div_up((n), (sz)) * (sz))

#define jd_array_too_big(fixed, obj, num) \
	((num) > (UINT_MAX - (fixed)) / (obj))

/*
 * Sector offset taken relative to the start of the target instead of
 * relative to the start of the device.
 */
#define jd_target_offset(ti, sector) ((sector) - (ti)->begin)

static inline sector_t to_sector(unsigned long n)
{
	return (n >> SECTOR_SHIFT);
}

static inline unsigned long to_bytes(sector_t n)
{
	return (n << SECTOR_SHIFT);
}

#endif	/* _LINUX_DEVICE_MAPPER_H */
