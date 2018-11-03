/*
 * args.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

arg(ARG_UNUSED, '-', "", 0, 0, 0, NULL)  /* place holder for unused 0 value */

arg(size_ARG, 'L', "size", sizemb_VAL, 0, 0,
    "#lbdself\n"
    "--create to build a lbd.\n"
    "specifies the size of the new lbd.\n"
    "the --size and --extents options are alternate methods of specifying size.\n")

arg(ARG_COUNT, '-', "", 0, 0, 0, NULL)
