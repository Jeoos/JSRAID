/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2014 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
