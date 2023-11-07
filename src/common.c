/* gbsddialog */
/* common.c */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021-2023 Alfonso Sabato Siciliano
 * Copyright (c) 2023 The FreeBSD Foundation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */



#include <stdarg.h>
#include <stdbool.h>
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
/* custom_text */
void custom_text(struct options * opt, char * text, char * buf)
{
	bool trim, crwrap;
	int i, j;

	if (strstr(text, "\\n") == NULL) {
		/* "hasnl" mode */
		trim = true;
		crwrap = true;
	} else {
		trim = false;
		crwrap = opt->cr_wrap;
	}
	if (opt->text_unchanged) {
		trim = false;
		crwrap = true;
	}

	for (i = 0, j = 0; text[i] != '\0'; i++) {
		switch (text[i]) {
			case '\\':
				buf[j] = '\\';
				switch (text[i+1]) {
					case 'n': /* implicitly in "hasnl" mode */
						buf[j] = '\n';
						i++;
						if (text[i+1] == '\n')
							i++;
						break;
					case 't':
						if (opt->tab_escape) {
							buf[j] = '\t';
						} else {
							j++;
							buf[j] = 't';
						}
						i++;
						break;
				}
				break;
			case '\n':
				buf[j] = crwrap ? '\n' : ' ';
				break;
			case '\t':
				buf[j] = opt->text_unchanged ? '\t' : ' ';
				break;
			default:
				buf[j] = text[i];
		}
		if (!trim || buf[j] != ' ' || j == 0 || buf[j-1] != ' ')
			j++;
	}
	buf[j] = '\0';
}


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
