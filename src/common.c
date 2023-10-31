/* gbsddialog */
/* common.c */



#include <stdarg.h>
#include <stdio.h>
#include "common.h"


/* variables */
struct exitcode exitcodes[] =
{
	{ "BSDDIALOG_ERROR",    255 },
	{ "BSDDIALOG_OK",         0 },
	{ "BSDDIALOG_CANCEL",     1 },
	{ "BSDDIALOG_HELP",       2 },
	{ "BSDDIALOG_EXTRA",      3 },
	{ "BSDDIALOG_TIMEOUT",    4 },
	{ "BSDDIALOG_ESC",        5 },
	{ "BSDDIALOG_LEFT1",      6 },
	{ "BSDDIALOG_LEFT2",      7 },
	{ "BSDDIALOG_LEFT3",      8 },
	{ "BSDDIALOG_RIGHT1",     9 },
	{ "BSDDIALOG_RIGHT2",    10 },
	{ "BSDDIALOG_RIGHT3",    11 },
	{ "BSDDIALOG_ITEM_HELP",  2 } /* like HELP by default */
};


/* functions */
/* error */
int error(int ret, char const * format, ...)
{
	va_list ap;

	fprintf(stderr, "%s: ", PROGNAME);
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}
