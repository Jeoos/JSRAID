/*
 * dev_manager.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __DEV_MANAGER_H__
#define __DEV_MANAGER_H__

struct cmd_context;
struct dev_manager;
struct lbd_activate_opts;
struct logical_block_device;

struct dev_manager *dev_manager_create(struct cmd_context *cmd,
				       const char *lp_name,
				       unsigned track_dvmove_deps);

int dev_manager_activate(struct dev_manager *dm, const struct logical_block_device *lbd,
			 struct lbd_activate_opts *laopts);

int dev_manager_deactivate(struct dev_manager *dm, const struct logical_block_device *lbd);
#endif
