/* gbsddialog */
/* common.c */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021-2023 Alfonso Sabato Siciliano
 * Copyright (c) 2023-2024 The FreeBSD Foundation
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



#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
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
void custom_text(struct options * opt, char const * text, char * buf)
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
	fputc('\n', stderr);
	va_end(ap);
	return ret;
}


/* get_font_size */
gdouble get_font_size(GdkScreen * screen)
{
#if GTK_CHECK_VERSION(3, 0, 0)
	GtkStyleContext * style;
	PangoFontDescription const * fontdesc;
	PangoFontMask mask;
#endif
	int fontsize;
	gdouble ex, resolution;

#if GTK_CHECK_VERSION(3, 0, 0)
	style = gtk_style_context_new();
	fontdesc = gtk_style_context_get_font(style, GTK_STATE_FLAG_NORMAL);
	mask = pango_font_description_get_set_fields(fontdesc);
	if((mask & PANGO_FONT_MASK_SIZE) == 0
			|| (fontsize = pango_font_description_get_size(
					fontdesc)) == 0)
		fontsize = 9 * PANGO_SCALE;
	else if(pango_font_description_get_size_is_absolute(fontdesc))
	{
		g_object_unref(style);
		return fontsize;
	}
	g_object_unref(style);
#else
	/* FIXME implement for Gtk+ 2 */
	fontsize = 9 * PANGO_SCALE;
#endif
	if((resolution = gdk_screen_get_resolution(screen)) < 0.0)
		resolution = 96.0;
	ex = (fontsize * resolution) / (72.0 * PANGO_SCALE);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() fontsize=%d resolution=%f ex=%f\n",
			__func__, fontsize, resolution, ex);
#endif
	return ex;
}


/* get_workarea */
void get_workarea(GdkScreen * screen, GdkRectangle * workarea)
{
#if GTK_CHECK_VERSION(3, 22, 0)
	GdkDisplay * display;

	display = gdk_screen_get_display(screen);
	gdk_monitor_get_workarea(gdk_display_get_primary_monitor(display),
			workarea);
#elif GTK_CHECK_VERSION(3, 4, 0)
	gdk_screen_get_monitor_workarea(screen, gdk_screen_get_primary_monitor(
				screen), workarea);
#else
	/* XXX this ignores window hints from any panels */
	gdk_screen_get_monitor_geometry(screen, gdk_screen_get_primary_monitor(
				screen), workarea);
#endif
}


/* init_entropy */
void init_entropy(void)
{
	srandom(time(NULL) ^ getpid() ^ getuid());
}


/* init_exitcodes */
void init_exitcodes(void)
{
	size_t i;
	char const * p;
	int v;

	for(i = 0; i < sizeof(exitcodes) / sizeof(*exitcodes); i++)
	{
		if((p = getenv(exitcodes[i].name)) == NULL)
			continue;
		errno = 0;
		v = strtol(p, NULL, 10);
		if(errno == 0)
			exitcodes[i].value = v;
	}
}


/* string_needs_quoting */
int string_needs_quoting(char const * str)
{
	char const * p;

	for(p = str; *p != '\0'; p++)
		switch(*p)
		{
			case '|':
			case '&':
			case ';':
			case '<':
			case '>':
			case '(':
			case ')':
			case '$':
			case '`':
			case '\\':
			case '"':
			case '\'':
			case ' ':
			case '\t':
			case '\n':
				return 1;
		}
	return 0;
}
