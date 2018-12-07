/*
 * xlate.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __XLATE_H__
#define __XLATE_H__

#ifdef __linux__
#  include <endian.h>
#  include <byteswap.h>
#else
#  include <machine/endian.h>
#  define bswap_16(x) (((x) & 0x00ffU) << 8 | \
		       ((x) & 0xff00U) >> 8)
#  define bswap_32(x) (((x) & 0x000000ffU) << 24 | \
		       ((x) & 0xff000000U) >> 24 | \
		       ((x) & 0x0000ff00U) << 8  | \
		       ((x) & 0x00ff0000U) >> 8)
#  define bswap_64(x) (((x) & 0x00000000000000ffULL) << 56 | \
		       ((x) & 0xff00000000000000ULL) >> 56 | \
		       ((x) & 0x000000000000ff00ULL) << 40 | \
		       ((x) & 0x00ff000000000000ULL) >> 40 | \
		       ((x) & 0x0000000000ff0000ULL) << 24 | \
		       ((x) & 0x0000ff0000000000ULL) >> 24 | \
		       ((x) & 0x00000000ff000000ULL) << 8 | \
		       ((x) & 0x000000ff00000000ULL) >> 8)
#endif

#if BYTE_ORDER == LITTLE_ENDIAN
/* new clearer variants. */
#define le16_to_cpu(x) (x)
#define le32_to_cpu(x) (x)
#define le64_to_cpu(x) (x)
#define cpu_to_le16(x) (x)
#define cpu_to_le32(x) (x)
#define cpu_to_le64(x) (x)
#define be16_to_cpu(x) bswap_16(x)
#define be32_to_cpu(x) bswap_32(x)
#define be64_to_cpu(x) bswap_64(x)
#define cpu_to_be16(x) bswap_16(x)
#define cpu_to_be32(x) bswap_32(x)
#define cpu_to_be64(x) bswap_64(x)
/* old alternative variants. */
#define xlate16(x) (x)
#define xlate32(x) (x)
#define xlate64(x) (x)
#define xlate16_be(x) bswap_16(x)
#define xlate32_be(x) bswap_32(x)
#define xlate64_be(x) bswap_64(x)

#elif BYTE_ORDER == BIG_ENDIAN
/* new clearer variants. */
#define le16_to_cpu(x) bswap_16(x)
#define le32_to_cpu(x) bswap_32(x)
#define le64_to_cpu(x) bswap_64(x)
#define cpu_to_le16(x) bswap_16(x)
#define cpu_to_le32(x) bswap_32(x)
#define cpu_to_le64(x) bswap_64(x)
#define be16_to_cpu(x) (x)
#define be32_to_cpu(x) (x)
#define be64_to_cpu(x) (x)
#define cpu_to_be16(x) (x)
#define cpu_to_be32(x) (x)
#define cpu_to_be64(x) (x)
/* old alternative variants. */
#define xlate16(x) bswap_16(x)
#define xlate32(x) bswap_32(x)
#define xlate64(x) bswap_64(x)
#define xlate16_be(x) (x)
#define xlate32_be(x) (x)
#define xlate64_be(x) (x)

#else
#include <asm/byteorder.h>
/* new clearer variants. */
#define le16_to_cpu(x) __le16_to_cpu(x)
#define le32_to_cpu(x) __le32_to_cpu(x)
#define le64_to_cpu(x) __le64_to_cpu(x)
#define cpu_to_le16(x) __cpu_to_le16(x)
#define cpu_to_le32(x) __cpu_to_le32(x)
#define cpu_to_le64(x) __cpu_to_le64(x)
#define be16_to_cpu(x) __be16_to_cpu(x)
#define be32_to_cpu(x) __be32_to_cpu(x)
#define be64_to_cpu(x) __be64_to_cpu(x)
#define cpu_to_be16(x) __cpu_to_be16(x)
#define cpu_to_be32(x) __cpu_to_be32(x)
#define cpu_to_be64(x) __cpu_to_be64(x)
/* old alternative variants. */
#define xlate16(x) __cpu_to_le16(x)
#define xlate32(x) __cpu_to_le32(x)
#define xlate64(x) __cpu_to_le64(x)
#define xlate16_be(x) __cpu_to_be16(x)
#define xlate32_be(x) __cpu_to_be32(x)
#define xlate64_be(x) __cpu_to_be64(x)
#endif

#endif
