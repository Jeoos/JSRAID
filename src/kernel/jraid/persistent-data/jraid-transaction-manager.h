/* 
 * jraid-transaction-manager.h
 *
 * Copyright(C)  2019
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __JRAID_TRANSACTION_MANAGER_H__
#define __JRAID_TRANSACTION_MANAGER_H__

struct jd_transaction_manager;
struct jd_block_manager;
struct jd_space_map;
struct jd_block_validator;

struct jd_block_manager *jd_tm_get_bm(struct jd_transaction_manager *tm);

int jd_tm_new_block(struct jd_transaction_manager *tm,
		    struct jd_block_validator *v,
		    struct jd_block **result);
void jd_tm_inc(struct jd_transaction_manager *tm, jd_block_t b);
void jd_tm_dec(struct jd_transaction_manager *tm, jd_block_t b);
int jd_tm_shadow_block(struct jd_transaction_manager *tm, jd_block_t orig,
		       struct jd_block_validator *v, struct jd_block **result,
		       int *inc_children);
void jd_tm_unlock(struct jd_transaction_manager *tm, struct jd_block *b);
#endif
