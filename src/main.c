/* gbsddialog */
/* main.c */
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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "common.h"
#include "gbsddialog.h"

#ifndef PROGNAME_BSDDIALOG
# define PROGNAME_BSDDIALOG	"bsddialog"
#endif
#ifndef PROGNAME_GBSDDIALOG
# define PROGNAME_GBSDDIALOG	"gbsddialog"
#endif


/* prototypes */
static int _usage(void);


/* functions */
/* main */
int main(int argc, char * argv[])
{
	int ret = 0, r;
	int i;
	char const * p;

	if((p = getenv("DISPLAY")) == NULL
			|| strlen(p) == 0)
	{
		execvp(PROGNAME_BSDDIALOG, argv);
		_exit(127);
	}

	if(setlocale(LC_ALL, "") == NULL)
		error(errno, "%s\n", strerror(errno));
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	gtk_init(&argc, &argv);

	for(i = 0; i < argc; i++)
	{
		if(strcmp(argv[i], "--version") == 0)
		{
			printf("Version: %s (libbsddialog: %s)\n",
					VERSION, LIBBSDDIALOG_VERSION);
			return (BSDDIALOG_OK);
		}
#ifdef WITH_XDIALOG
		if(strcmp(argv[i], "--usage") == 0)
#else
		if(strcmp(argv[i], "--help") == 0)
#endif
		{
			_usage();
			return (BSDDIALOG_OK);
		}
	}

	if((r = gbsddialog(&ret, argc, (char const **)argv)) != 0)
		return r;

	gtk_main();

	return ret;
}


/* usage */
static int _usage(void)
{
#ifdef WITH_XDIALOG
        printf("usage: " PROGNAME_GBSDDIALOG " --usage | --version\n");
#else
        printf("usage: " PROGNAME_GBSDDIALOG " --help | --version\n");
#endif
        printf("       " PROGNAME_GBSDDIALOG " [--<opt>] --<dialog> <text> <rows> <cols> "
            "[<arg>] [--<opt>]\n");
        printf("       " PROGNAME_GBSDDIALOG " ... --<dialog1> ... [--and-dialog --<dialog2> "
            "...] ...\n");
        printf("\n");

        printf("Options:\n");
        printf(" --alternate-screen, --ascii-lines, --backtitle <backtitle>,"
            " --begin-x <x>,\n --begin-y <y>, --bikeshed,"
            " --cancel-exit-code <retval>, --cancel-label <label>,\n"
            " --clear-dialog, --clear-screen, --columns-per-row <columns>,"
            " --cr-wrap,\n --datebox-format d/m/y|m/d/y|y/m/d,"
            " --date-format <format>,\n --default-button <label>,"
            " --default-item <name>, --default-no, --disable-esc,\n"
            " --error-exit-code <retval>, --esc-exit-code <retval>,"
            " --exit-label <label>,\n --extra-button,"
            " --extra-exit-code <retval>, --extra-label <label>,\n"
            " --left1-button <label>, --left1-exit-code <retval>,"
            " --left2-button <label>,\n --left2-exit-code <retval>,"
            " --left3-button <label>, --left3-exit-code <retval>,\n"
#ifdef WITH_XDIALOG
	    " --help <text>,\n"
#endif
            " --help-button, --help-exit-code <retval>, --help-label <label>,\n"
            " --help-print-items, --help-print-name, --hfile <file>,"
            " --hline <string>,\n --hmsg <string>, --ignore, --insecure,"
            " --item-bottom-desc, --item-depth,\n --item-prefix,"
            " --load-theme <file>, --max-input <size>, --no-cancel,\n"
            " --no-descriptions, --no-label <label>, --no-lines, --no-names,"
            " --no-ok,\n --no-shadow, --normal-screen, --ok-exit-code <retval>,"
            " --ok-label <label>,\n --output-fd <fd>, --output-separator <sep>,"
            " --print-maxsize, --print-size,\n --print-version, --quoted,"
            " --right1-button <label>,\n --right1-exit-code <retval>,"
            " --right2-button <label>,\n --right2-exit-code <retval>,"
            " --right3-button <label>,\n --right3-exit-code <retval>,"
            " --save-theme <file>, --separate-output,\n --separator <sep>,"
            " --shadow, --single-quoted, --sleep <secs>, --stderr,\n --stdout,"
            " --switch-buttons, --tab-escape, --tab-len <spaces>,"
            " --text-escape,\n --text-unchanged, --theme 3d|blackwhite|flat,"
            " --timeout-exit-code <retval>,\n --time-format <format>,"
            " --title <title>, --yes-label <label>.");
        printf("\n\n");

        printf("Dialogs:\n");
        printf(" --calendar <text> <rows> <cols> [<dd> <mm> <yy>]\n");
        printf(" --checklist <text> <rows> <cols> <menurows> [<name> <desc> "
            "on|off] ...\n");
        printf(" --datebox <text> <rows> <cols> [<dd> <mm> <yy>]\n");
        printf(" --form <text> <rows> <cols> <formrows> [<label> <ylabel> "
            "<xlabel> <init> <yfield> <xfield> <fieldlen> <maxletters>] "
            "...\n");
        printf(" --gauge <text> <rows> <cols> [<perc>]\n");
        printf(" --infobox <text> <rows> <cols>\n");
        printf(" --inputbox <text> <rows> <cols> [<init>]\n");
        printf(" --menu <text> <rows> <cols> <menurows> [<name> <desc>] ...\n");
        printf(" --mixedform <text> <rows> <cols> <formrows> [<label> <ylabel> "
            "<xlabel> <init> <yfield> <xfield> <fieldlen> <maxletters> "
            "0|1|2] ...\n");
        printf(" --mixedgauge <text> <rows> <cols> <mainperc> [<minilabel> "
            "<miniperc>] ...\n");
        printf(" --msgbox <text> <rows> <cols>\n");
        printf(" --passwordbox <text> <rows> <cols> [<init>]\n");
        printf(" --passwordform <text> <rows> <cols> <formrows> [<label> "
            "<ylabel> <xlabel> <init> <yfield> <xfield> <fieldlen> "
            "<maxletters>] ...\n");
        printf(" --pause <text> <rows> <cols> <secs>\n");
        printf(" --radiolist <text> <rows> <cols> <menurows> [<name> <desc> "
            "on|off] ...\n");
        printf(" --rangebox <text> <rows> <cols> <min> <max> [<init>]\n");
        printf(" --textbox <file> <rows> <cols>\n");
        printf(" --timebox <text> <rows> <cols> [<hh> <mm> <ss>]\n");
        printf(" --treeview <text> <rows> <cols> <menurows> [<depth> <name> "
            "<desc> on|off] ...\n");
        printf(" --yesno <text> <rows> <cols>\n");
        printf("\n");

        printf("See 'man 1 bsddialog' for more information.\n");

	return 0;
}
