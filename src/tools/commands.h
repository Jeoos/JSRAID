/*
 * commands.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

xx(version,
   "Display software and driver version information",
   PERMITTED_READ_ONLY | NO_METADATA_PROCESSING)

xx(dvcreate,
   "Initialize disk volume(s) for use by lbd",
   ENABLE_ALL_DEVS)

xx(dvremove,
   "Remove lbd label(s) from disk volume(s)",
   ENABLE_ALL_DEVS)

xx(lbdpool,
   "For lbd pool functions",
   ENABLE_ALL_DEVS)

xx(lbdself,
   "For kinds of lbd itself functions",
   ENABLE_ALL_DEVS)
