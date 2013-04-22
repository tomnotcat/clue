#include <glib.h>

#define VERSION "1.0.0"

#ifdef G_OS_WIN32
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif
