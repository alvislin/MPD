/* the Music Player Daemon (MPD)
 * Copyright (C) 2003-2007 by Warren Dukes (warren.dukes@gmail.com)
 * This project's homepage is: http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "permission.h"

#include "conf.h"
#include "log.h"
#include "utils.h"
#include "os_compat.h"

#include <glib.h>
#include <stdbool.h>

#define PERMISSION_PASSWORD_CHAR	"@"
#define PERMISSION_SEPERATOR		","

#define PERMISSION_READ_STRING		"read"
#define PERMISSION_ADD_STRING		"add"
#define PERMISSION_CONTROL_STRING	"control"
#define PERMISSION_ADMIN_STRING		"admin"

static GHashTable *permission_passwords;

static unsigned permission_default;

static unsigned parsePermissions(char *string)
{
	unsigned permission = 0;
	char *temp;
	char *tok;

	if (!string)
		return 0;

	temp = strtok_r(string, PERMISSION_SEPERATOR, &tok);
	while (temp) {
		if (strcmp(temp, PERMISSION_READ_STRING) == 0) {
			permission |= PERMISSION_READ;
		} else if (strcmp(temp, PERMISSION_ADD_STRING) == 0) {
			permission |= PERMISSION_ADD;
		} else if (strcmp(temp, PERMISSION_CONTROL_STRING) == 0) {
			permission |= PERMISSION_CONTROL;
		} else if (strcmp(temp, PERMISSION_ADMIN_STRING) == 0) {
			permission |= PERMISSION_ADMIN;
		} else {
			FATAL("unknown permission \"%s\"\n", temp);
		}

		temp = strtok_r(NULL, PERMISSION_SEPERATOR, &tok);
	}

	return permission;
}

void initPermissions(void)
{
	char *temp;
	char *cp2;
	char *password;
	unsigned permission;
	ConfigParam *param;

	permission_passwords = g_hash_table_new_full(g_str_hash, g_str_equal,
						     g_free, NULL);

	permission_default = PERMISSION_READ | PERMISSION_ADD |
	    PERMISSION_CONTROL | PERMISSION_ADMIN;

	param = getNextConfigParam(CONF_PASSWORD, NULL);

	if (param) {
		permission_default = 0;

		do {
			if (!strstr(param->value, PERMISSION_PASSWORD_CHAR)) {
				FATAL("\"%s\" not found in password string "
				      "\"%s\", line %i\n",
				      PERMISSION_PASSWORD_CHAR,
				      param->value, param->line);
			}

			if (!(temp = strtok_r(param->value,
					      PERMISSION_PASSWORD_CHAR,
					      &cp2))) {
				FATAL("something weird just happened in permission.c\n");
			}

			password = temp;

			permission =
			    parsePermissions(strtok_r(NULL, "", &cp2));

			g_hash_table_replace(permission_passwords,
					     g_strdup(password),
					     GINT_TO_POINTER(permission));
		} while ((param = getNextConfigParam(CONF_PASSWORD, param)));
	}

	param = getConfigParam(CONF_DEFAULT_PERMS);

	if (param)
		permission_default = parsePermissions(param->value);
}

int getPermissionFromPassword(char *password, unsigned *permission)
{
	bool found;
	gpointer key, value;

	found = g_hash_table_lookup_extended(permission_passwords,
					     password, &key, &value);
	if (!found)
		return -1;

	*permission = GPOINTER_TO_INT(value);
	return 0;
}

void finishPermissions(void)
{
	g_hash_table_destroy(permission_passwords);
}

unsigned getDefaultPermissions(void)
{
	return permission_default;
}
