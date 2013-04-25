/* Ctk - The Clue Toolkit
 *
 * Copyright (C) 2013 Tom Wong
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "config.h"
#include "ctkfileutils.h"
#include <string.h>

static void _test_mime_types (void)
{
    gchar *uri;
    gchar *mime;

    uri = g_filename_to_uri (TEST_TOP_SRCDIR "test.pdf", NULL, NULL);
    mime = ctk_file_get_mime_type (uri, TRUE, NULL);
    g_assert (!strcmp (mime, "application/pdf"));
    g_free (uri);
    g_free (mime);

    uri = g_filename_to_uri (TEST_TOP_SRCDIR "test.pdf", NULL, NULL);
    mime = ctk_file_get_mime_type (uri, FALSE, NULL);
    g_assert (!strcmp (mime, "application/pdf"));
    g_free (uri);
    g_free (mime);

    uri = g_filename_to_uri (TEST_TOP_SRCDIR "test.txt", NULL, NULL);
    mime = ctk_file_get_mime_type (uri, TRUE, NULL);
    g_assert (!strcmp (mime, "text/plain"));
    g_free (uri);
    g_free (mime);

    uri = g_filename_to_uri (TEST_TOP_SRCDIR "test.txt", NULL, NULL);
    mime = ctk_file_get_mime_type (uri, FALSE, NULL);
    g_assert (!strcmp (mime, "text/plain"));
    g_free (uri);
    g_free (mime);
}

int main (int argc, char *argv[])
{
    g_type_init ();

    _test_mime_types ();

    return 0;
}
