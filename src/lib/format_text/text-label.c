/*
 * text_label.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "label.h"
#include "device.h"
#include "format-text.h"

#include <string.h>
static int _text_initialise_label(struct labeller *l __attribute__((unused)),
				  struct label *label)
{
	strncpy(label->type, LBD_LABEL, sizeof(label->type));

	return 1;
}

static int _text_write(struct label *label, void *buf)
{
	struct label_header *lh = (struct label_header *) buf;

	strncpy(label->type, LBD_LABEL, sizeof(label->type));
	strncpy((char *)lh->type, label->type, sizeof(label->type));
        return 1;
}

static int _text_read(struct labeller *l, struct device *dev, void *label_buf,
		      struct label **label)
{
        return 1;
}

struct label_ops _text_ops = {
	.write = _text_write,
	.read = _text_read,
	.initialise_label = _text_initialise_label,
};

struct labeller *text_labeller_create(const struct format_type *fmt)
{
	struct labeller *l;

	if (!(l = malloc(sizeof(*l)))) {
		printf("couldn't allocate labeller object.\n");
		return NULL;
	}

	l->ops = &_text_ops;
	l->fmt = fmt;

	return l;
}
