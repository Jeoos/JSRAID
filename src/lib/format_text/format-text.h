/*
 * format-text.h
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __FORMAT_TEXT_H__
#define __FORMAT_TEXT_H__

#include "metadata.h"

#define FMT_TEXT_NAME "lbd"
#define LBD_LABEL "LBD  001"

struct format_type *create_text_format(struct cmd_context *cmd);
struct labeller *text_labeller_create(const struct format_type *fmt);

#endif
