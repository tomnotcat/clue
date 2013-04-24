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
#include "ctkfileutils.h"
#include <gio/gio.h>

static gchar* get_mime_type_from_uri (const gchar *uri, GError **error)
{
    GFile *file;
    GFileInfo *file_info;
    const gchar *content_type;
    gchar *mime_type = NULL;

    file = g_file_new_for_uri (uri);
    file_info = g_file_query_info (file,
                                   G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                   0, NULL, error);
    g_object_unref (file);

    if (file_info == NULL)
        return NULL;

    content_type = g_file_info_get_content_type (file_info);
    if (content_type) {
        mime_type = g_content_type_get_mime_type (content_type);
    }

    g_object_unref (file_info);
    return mime_type;
}

static gchar* get_mime_type_from_data (const gchar *uri, GError **error)
{
    GFile *file;
    GFileInputStream *input_stream;
    gssize size_read;
    guchar buffer[1024];
    gboolean retval;
    gchar *content_type, *mime_type;

    file = g_file_new_for_uri (uri);

    input_stream = g_file_read (file, NULL, error);
    if (!input_stream) {
        g_object_unref (file);
        return NULL;
    }

    size_read = g_input_stream_read (G_INPUT_STREAM (input_stream),
                                     buffer, sizeof (buffer), NULL, error);
    if (size_read == -1) {
        g_object_unref (input_stream);
        g_object_unref (file);
        return NULL;
    }

    retval = g_input_stream_close (G_INPUT_STREAM (input_stream), NULL, error);

    g_object_unref (input_stream);
    g_object_unref (file);
    if (!retval)
        return NULL;

    content_type = g_content_type_guess (NULL, /* no filename */
                                         buffer, size_read,
                                         NULL);
    if (!content_type)
        return NULL;

#ifdef G_OS_WIN32
    /* On Windows, the implementation of g_content_type_guess() is-
     * sometimes too limited, so we do use get_mime_type_from_uri()-
     * as a fallback */
    if (strcmp (content_type, "*") == 0) {
        g_free (content_type);
        return get_mime_type_from_uri (uri, error);
    }
#endif /* G_OS_WIN32 */

    mime_type = g_content_type_get_mime_type (content_type);
    g_free (content_type);
    return mime_type;
}

/**
 * ctk_file_get_mime_type:
 * @uri: the URI
 * @fast: whether to use fast MIME type detection
 * @error: a #GError location to store an error, or %NULL
 *
 * Note: on unknown MIME types, this may return NULL without @error
 * being filled in.
 *-
 * Returns: a newly allocated string with the MIME type of the file at
 *   @uri, or %NULL on error or if the MIME type could not be determined
 */
gchar* ctk_file_get_mime_type (const gchar *uri,
                               gboolean fast,
                               GError **error)
{
    return fast ? get_mime_type_from_uri (uri, error) :
            get_mime_type_from_data (uri, error);
}
