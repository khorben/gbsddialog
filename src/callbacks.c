/* gbsddialog */
/* callbacks.c */



#include <stdlib.h>
#include <stdio.h>
#include <bsddialog.h>
#include "common.h"
#include "callbacks.h"


/* error_args */
void error_args(char const * dialog, int argc, char const ** argv)
{
	int i;

	printf("Error: %s unexpected argument%s:", dialog, argc > 1 ? "s" : "");
	for (i = 0; i < argc; i++)
		printf(" \"%s\"", argv[i]);
	printf(".\n\n");
	printf("See \'gbsddialog --help\' or \'man 1 bsddialog\' ");
	printf("for more information.\n");

	exit(EXITCODE(BSDDIALOG_ERROR));
}
