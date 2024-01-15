/* gbsddialog */
/* common.h */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
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



#ifndef GBSDDIALOG_COMMON_H
# define GBSDDIALOG_COMMON_H

# include <stdbool.h>
# include <bsddialog.h>


/* common */
/* macros */
# define PACKAGE	"gbsddialog"
# define VERSION	"0.0.1"

# ifndef PROGNAME
#  define PROGNAME	PACKAGE
# endif

# ifndef PREFIX
#  define PREFIX	"/usr/local"
# endif
# ifndef DATADIR
#  define DATADIR	PREFIX "/share"
# endif
# ifndef LOCALEDIR
#  define LOCALEDIR	DATADIR "/locale"
# endif

# define DEFAULT_COLS_PER_ROW	10

# define EXITCODE(retval)	(exitcodes[retval + 1].value)


/* types */
enum bsddialog_default_theme
{
	BSDDIALOG_THEME_3D,
	BSDDIALOG_THEME_BLACKWHITE,
	BSDDIALOG_THEME_FLAT
};

struct exitcode
{
	const char * name;
	int value;
};

struct options
{
	/* Menus options */
	bool item_always_quote;
	char *item_default;
	bool item_depth;
	char *item_output_sep;
	bool item_output_sepnl;
	bool item_prefix;
	bool item_singlequote;
	/* Menus and Forms options */
	bool help_print_item_name;
	bool help_print_items;
	bool item_bottomdesc;
	/* Forms options */
	int unsigned max_input_form;
#ifdef WITH_XDIALOG
	/* Help */
	char * help;
#endif
	/* Date and Time options */
	char *date_fmt;
	char *time_fmt;
	/* General options */
	int getH;
	int getW;
	bool ignore;
	bool ignore_eof;
	int output_fd;
	/* Text option */
	bool cr_wrap;
	bool tab_escape;
	bool text_unchanged;
	/* Theme and Screen options*/
	char *backtitle;
	bool bikeshed;
	enum bsddialog_default_theme theme;
	bool clearscreen;
	char *loadthemefile;
	char *savethemefile;
	const char *screen_mode;
	/* Dialog */
	bool mandatory_dialog;
#ifdef WITH_XDIALOG
	bool without_buttons;
#endif
	const char *name;
	int (*dialogbuilder)(struct bsddialog_conf const * conf,
	    char const * text, int rows, int cols,
	    int argc, char const ** argv, struct options const * opt);
};


/* variables */
extern struct exitcode exitcodes[];


/* functions */
void custom_text(struct options * opt, char const * text, char * buf);

int error(int ret, char const * format, ...);

int string_needs_quoting(char const * str);

#endif /* !GBSDDIALOG_COMMON_H */
