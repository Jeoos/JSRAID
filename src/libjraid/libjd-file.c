/*
 * libjd-file.c
 *
 * Contact: JeCortex@yahoo.com
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include "libjraid.h"

static int _is_dir(const char *path)
{
	struct stat st;

	if (stat(path, &st) < 0) {
		printf("stat %s\n", path);
		return 0;
	}

	if (!S_ISDIR(st.st_mode)) {
		printf("Existing path %s is not "
			  "a directory.", path);
		return 0;
	}

	return 1;
}

static int _create_dir_recursive(const char *dir)
{
	char *orig, *s;
	int rc, r = 0;

	printf("Creating directory \"%s\"", dir);
	/* Create parent directories */
	orig = s = strdup(dir);
	if (!s) {
		printf("Failed to duplicate directory name.\n");
		return 0;
	}

	while ((s = strchr(s, '/')) != NULL) {
		*s = '\0';
		if (*orig) {
			rc = mkdir(orig, 0777);
			if (rc < 0) {
				if (errno == EEXIST) {
					if (!_is_dir(orig))
						goto out;
				} else {
					if (errno != EROFS)
						printf("mkdir %s\n", orig);
					goto out;
				}
			}
		}
		*s++ = '/';
	}

	/* Create final directory */
	rc = mkdir(dir, 0777);
	if (rc < 0) {
		if (errno == EEXIST) {
			if (!_is_dir(dir))
				goto out;
		} else {
			if (errno != EROFS)
				printf("mkdir %s\n", orig);
			goto out;
		}
	}

	r = 1;
out:
	return r;
}

int jd_create_dir(const char *dir)
{
	struct stat info;

	if (!*dir)
		return 1;

	if (stat(dir, &info) == 0 && S_ISDIR(info.st_mode))
		return 1;

	if (!_create_dir_recursive(dir))
		return 0;

	return 1;
}
