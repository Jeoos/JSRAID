/*
 * locking.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

 #ifndef __LOCKING_H__
 #define __LOCKING_H__

#define lock_lbd(cmd, lbd, flags) \
({ \
	int rr = 0; \
\
        /* FIXME */ \
\
	rr; \
})

#define activate_lbd_excl_local(cmd, lbd)        \
                lock_lbd(cmd, lbd, flags)

 #endif
