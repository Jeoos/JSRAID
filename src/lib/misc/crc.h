/*
 * crc.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __CRC_H__
#define __CRC_H__

#include <inttypes.h>

#define INITIAL_CRC 0xf597a6cf

uint32_t calc_crc(uint32_t initial, const uint8_t *buf, uint32_t size);

#endif
