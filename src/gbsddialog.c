/* gbsddialog */
/* gbsddialog.c */
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
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <getopt.h>
#include "builders.h"
#include "common.h"
#include "gbsddialog.h"

/* FIXME conflicts with <sys/syslimits.h> */
#ifdef MAX_INPUT
# undef MAX_INPUT
#endif


/* private */
/* types */
typedef struct _GBSDDialog
{
	struct bsddialog_conf conf;
	struct options opt;

	int * ret;
	int argc;
	char const ** argv;

	/* for Gtk+ */
	GdkScreen * screen;

	/* for backtitle */
	char const * backtitle;
	GtkWidget ** windows;
	size_t windows_cnt;
	GtkWidget * label;
} GBSDDialog;

/* for getopt_long() */
enum OPTS {
	/* Options */
#ifdef WITH_XDIALOG
	ALLOW_CLOSE = '?' + 1,
	ALTERNATE_SCREEN,
#else
	ALTERNATE_SCREEN = '?' + 1,
#endif
	AND_DIALOG,
	ASCII_LINES,
	BACKTITLE,
	BEGIN_X,
	BEGIN_Y,
	BIKESHED,
	CANCEL_EXIT_CODE,
	CANCEL_LABEL,
	CLEAR_DIALOG,
	CLEAR_SCREEN,
	COLUMNS_PER_ROW,
	CR_WRAP,
	DATEBOX_FORMAT,
	DATE_FORMAT,
	DEFAULT_BUTTON,
	DEFAULT_ITEM,
	DEFAULT_NO,
	DISABLE_ESC,
#ifdef WITH_XDIALOG
	EDITABLE,
#endif
	ERROR_EXIT_CODE,
	ESC_EXIT_CODE,
	EXIT_LABEL,
	EXTRA_BUTTON,
	EXTRA_EXIT_CODE,
	EXTRA_LABEL,
#ifdef WITH_XDIALOG
	FIXED_FONT,
	HELP,
#endif
	HELP_BUTTON,
	HELP_EXIT_CODE,
	HELP_LABEL,
	HELP_PRINT_ITEMS,
	HELP_PRINT_NAME,
	HFILE,
	HLINE,
	HMSG,
#ifdef WITH_XDIALOG
	ICON,
#endif
	IGNORE,
#ifdef WITH_XDIALOG
	IGNORE_EOF,
#endif
	INSECURE,
	ITEM_BOTTOM_DESC,
	ITEM_DEPTH,
	ITEM_PREFIX,
	LEFT1_BUTTON,
	LEFT1_EXIT_CODE,
	LEFT2_BUTTON,
	LEFT2_EXIT_CODE,
	LEFT3_BUTTON,
	LEFT3_EXIT_CODE,
	LOAD_THEME,
	MAX_INPUT,
	NO_BUTTONS,
	NO_CANCEL,
#ifdef WITH_XDIALOG
	NO_CR_WRAP,
#endif
	NO_DESCRIPTIONS,
	NO_LINES,
	NO_NAMES,
	NO_OK,
	NO_SHADOW,
	NORMAL_SCREEN,
	OK_EXIT_CODE,
	OK_LABEL,
	OUTPUT_FD,
	OUTPUT_SEPARATOR,
	PRINT_MAXSIZE,
	PRINT_SIZE,
	PRINT_VERSION,
	QUOTED,
	RIGHT1_BUTTON,
	RIGHT1_EXIT_CODE,
	RIGHT2_BUTTON,
	RIGHT2_EXIT_CODE,
	RIGHT3_BUTTON,
	RIGHT3_EXIT_CODE,
	SAVE_THEME,
	SEPARATE_OUTPUT,
	SHADOW,
	SINGLE_QUOTED,
	SLEEP,
	STDERR,
	STDOUT,
	SWITCH_BUTTONS,
	TAB_ESCAPE,
	TAB_LEN,
	TEXT_ESCAPE,
	TEXT_UNCHANGED,
	THEME,
	TIMEOUT_EXIT_CODE,
	TIME_FORMAT,
	TITLE,
#ifdef WITH_XDIALOG
	WIZARD,
#endif
	/* Dialogs */
	CALENDAR,
	CHECKLIST,
#ifdef WITH_XDIALOG
	COLORSEL,
	COMBOBOX,
#endif
	DATEBOX,
#ifdef WITH_XDIALOG
	DSELECT,
	FONTSEL,
#endif
	FORM,
#ifdef WITH_XDIALOG
	FSELECT,
#endif
	GAUGE,
	INFOBOX,
	INPUTBOX,
#ifdef WITH_XDIALOG
	INPUTSBOX2,
	INPUTSBOX3,
#endif
	MENU,
	MIXEDFORM,
	MIXEDGAUGE,
	MSGBOX,
	PASSWORDBOX,
	PASSWORDFORM,
	PAUSE,
	RADIOLIST,
	RANGEBOX,
#ifdef WITH_XDIALOG
	RANGESBOX2,
	RANGESBOX3,
#endif
	TEXTBOX,
	TIMEBOX,
	TREEVIEW,
	YESNO
};


/* variables */
static struct option longopts[] = {
	/* Options */
#ifdef WITH_XDIALOG
	{"allow-close",	      no_argument,	 NULL, ALLOW_CLOSE},
#endif
	{"alternate-screen",  no_argument,       NULL, ALTERNATE_SCREEN},
	{"and-dialog",        no_argument,       NULL, AND_DIALOG},
	{"and-widget",        no_argument,       NULL, AND_DIALOG},
	{"ascii-lines",       no_argument,       NULL, ASCII_LINES},
	{"backtitle",         required_argument, NULL, BACKTITLE},
	{"begin-x",           required_argument, NULL, BEGIN_X},
	{"begin-y",           required_argument, NULL, BEGIN_Y},
	{"bikeshed",          no_argument,       NULL, BIKESHED},
	{"cancel-exit-code",  required_argument, NULL, CANCEL_EXIT_CODE},
	{"cancel-label",      required_argument, NULL, CANCEL_LABEL},
	{"clear",             no_argument,       NULL, CLEAR_SCREEN},
	{"clear-dialog",      no_argument,       NULL, CLEAR_DIALOG},
	{"clear-screen",      no_argument,       NULL, CLEAR_SCREEN},
	{"colors",            no_argument,       NULL, TEXT_ESCAPE},
#if 0
	{"columns-per-row",   required_argument, NULL, COLUMNS_PER_ROW},
#endif
	{"cr-wrap",           no_argument,       NULL, CR_WRAP},
#if 0
	{"datebox-format",    required_argument, NULL, DATEBOX_FORMAT},
#endif
	{"date-format",       required_argument, NULL, DATE_FORMAT},
	{"defaultno",         no_argument,       NULL, DEFAULT_NO},
#if 0
	{"default-button",    required_argument, NULL, DEFAULT_BUTTON},
#endif
	{"default-item",      required_argument, NULL, DEFAULT_ITEM},
	{"default-no",        no_argument,       NULL, DEFAULT_NO},
	{"disable-esc",       no_argument,       NULL, DISABLE_ESC},
#ifdef WITH_XDIALOG
	{"editable",          no_argument,       NULL, EDITABLE},
#endif
	{"error-exit-code",   required_argument, NULL, ERROR_EXIT_CODE},
	{"esc-exit-code",     required_argument, NULL, ESC_EXIT_CODE},
	{"exit-label",        required_argument, NULL, EXIT_LABEL},
	{"extra-button",      no_argument,       NULL, EXTRA_BUTTON},
	{"extra-exit-code",   required_argument, NULL, EXTRA_EXIT_CODE},
	{"extra-label",       required_argument, NULL, EXTRA_LABEL},
#ifdef WITH_XDIALOG
	{"fixed-font",	      no_argument,       NULL, FIXED_FONT},
	{"help",	      required_argument, NULL, HELP},
#endif
	{"help-button",       no_argument,       NULL, HELP_BUTTON},
	{"help-exit-code",    required_argument, NULL, HELP_EXIT_CODE},
	{"help-label",        required_argument, NULL, HELP_LABEL},
#if 0
	{"help-print-items",  no_argument,       NULL, HELP_PRINT_ITEMS},
	{"help-print-name",   no_argument,       NULL, HELP_PRINT_NAME},
	{"help-status",       no_argument,       NULL, HELP_PRINT_ITEMS},
	{"help-tags",         no_argument,       NULL, HELP_PRINT_NAME},
#endif
	{"hfile",             required_argument, NULL, HFILE},
	{"hline",             required_argument, NULL, HLINE},
	{"hmsg",              required_argument, NULL, HMSG},
#ifdef WITH_XDIALOG
	{"icon",              required_argument, NULL, ICON},
#endif
	{"ignore",            no_argument,       NULL, IGNORE},
#ifdef WITH_XDIALOG
	{"ignore-eof",        no_argument,       NULL, IGNORE_EOF},
#endif
	{"insecure",          no_argument,       NULL, INSECURE},
	{"item-bottom-desc",  no_argument,       NULL, ITEM_BOTTOM_DESC},
	{"item-depth",        no_argument,       NULL, ITEM_DEPTH},
	{"item-help",         no_argument,       NULL, ITEM_BOTTOM_DESC},
	{"item-prefix",       no_argument,       NULL, ITEM_PREFIX},
	{"keep-tite",         no_argument,       NULL, ALTERNATE_SCREEN},
#if 0
	{"left1-button",      required_argument, NULL, LEFT1_BUTTON},
	{"left1-exit-code",   required_argument, NULL, LEFT1_EXIT_CODE},
	{"left2-button",      required_argument, NULL, LEFT2_BUTTON},
	{"left2-exit-code",   required_argument, NULL, LEFT2_EXIT_CODE},
	{"left3-button",      required_argument, NULL, LEFT3_BUTTON},
	{"left3-exit-code",   required_argument, NULL, LEFT3_EXIT_CODE},
#endif
	{"load-theme",        required_argument, NULL, LOAD_THEME},
	{"max-input",         required_argument, NULL, MAX_INPUT},
#ifdef WITH_XDIALOG
	{"no-buttons",        no_argument,       NULL, NO_BUTTONS},
#endif
	{"no-cancel",         no_argument,       NULL, NO_CANCEL},
	{"nocancel",          no_argument,       NULL, NO_CANCEL},
#ifdef WITH_XDIALOG
	{"no-close",	      no_argument,	 NULL, DISABLE_ESC},
	{"no-cr-wrap",	      no_argument,	 NULL, NO_CR_WRAP},
#endif
	{"no-descriptions",   no_argument,       NULL, NO_DESCRIPTIONS},
	{"no-items",          no_argument,       NULL, NO_DESCRIPTIONS},
	{"no-label",          required_argument, NULL, CANCEL_LABEL},
	{"no-lines",          no_argument,       NULL, NO_LINES},
	{"no-names",          no_argument,       NULL, NO_NAMES},
	{"no-ok",             no_argument,       NULL, NO_OK},
	{"nook",              no_argument,       NULL, NO_OK},
	{"no-shadow",         no_argument,       NULL, NO_SHADOW},
	{"no-tags",           no_argument,       NULL, NO_NAMES},
	{"normal-screen",     no_argument,       NULL, NORMAL_SCREEN},
	{"ok-exit-code",      required_argument, NULL, OK_EXIT_CODE},
	{"ok-label",          required_argument, NULL, OK_LABEL},
	{"output-fd",         required_argument, NULL, OUTPUT_FD},
	{"output-separator",  required_argument, NULL, OUTPUT_SEPARATOR},
#ifdef WITH_XDIALOG
	{"password",          no_argument,       NULL, INSECURE},
#endif
	{"print-maxsize",     no_argument,       NULL, PRINT_MAXSIZE},
	{"print-size",        no_argument,       NULL, PRINT_SIZE},
	{"print-version",     no_argument,       NULL, PRINT_VERSION},
	{"quoted",            no_argument,       NULL, QUOTED},
#if 0
	{"right1-button",     required_argument, NULL, RIGHT1_BUTTON},
	{"right1-exit-code",  required_argument, NULL, RIGHT1_EXIT_CODE},
	{"right2-button",     required_argument, NULL, RIGHT2_BUTTON},
	{"right2-exit-code",  required_argument, NULL, RIGHT2_EXIT_CODE},
	{"right3-button",     required_argument, NULL, RIGHT3_BUTTON},
	{"right3-exit-code",  required_argument, NULL, RIGHT3_EXIT_CODE},
#endif
	{"save-theme",        required_argument, NULL, SAVE_THEME},
	{"separate-output",   no_argument,       NULL, SEPARATE_OUTPUT},
	{"separator",         required_argument, NULL, OUTPUT_SEPARATOR},
	{"shadow",            no_argument,       NULL, SHADOW},
	{"single-quoted",     no_argument,       NULL, SINGLE_QUOTED},
	{"sleep",             required_argument, NULL, SLEEP},
	{"stderr",            no_argument,       NULL, STDERR},
	{"stdout",            no_argument,       NULL, STDOUT},
	{"switch-buttons",    no_argument,       NULL, SWITCH_BUTTONS},
	{"tab-escape",        no_argument,       NULL, TAB_ESCAPE},
#if 0
	{"tab-len",           required_argument, NULL, TAB_LEN},
#endif
	{"text-escape",       no_argument,       NULL, TEXT_ESCAPE},
	{"text-unchanged",    no_argument,       NULL, TEXT_UNCHANGED},
	{"theme",             required_argument, NULL, THEME},
	{"timeout-exit-code", required_argument, NULL, TIMEOUT_EXIT_CODE},
	{"time-format",       required_argument, NULL, TIME_FORMAT},
	{"title",             required_argument, NULL, TITLE},
#ifdef WITH_XDIALOG
	{"wizard",            no_argument,       NULL, WIZARD},
#endif
	{"yes-label",         required_argument, NULL, OK_LABEL},
	/* Dialogs */
#ifdef WITH_XDIALOG
	{ "2inputsbox",  no_argument, NULL, INPUTSBOX2},
	{ "3inputsbox",  no_argument, NULL, INPUTSBOX3},
	{ "2rangesbox",  no_argument, NULL, RANGESBOX2},
	{ "3rangesbox",  no_argument, NULL, RANGESBOX3},
#endif
	{"calendar",     no_argument, NULL, CALENDAR},
	{"checklist",    no_argument, NULL, CHECKLIST},
#ifdef WITH_XDIALOG
	{"colorsel",     no_argument, NULL, COLORSEL},
	{"combobox",     no_argument, NULL, COMBOBOX},
#endif
	{"datebox",      no_argument, NULL, DATEBOX},
#ifdef WITH_XDIALOG
	{"dselect",      no_argument, NULL, DSELECT},
#endif
	{"form",         no_argument, NULL, FORM},
#ifdef WITH_XDIALOG
	{"fontsel",      no_argument, NULL, FONTSEL},
	{"fselect",      no_argument, NULL, FSELECT},
#endif
	{"gauge",        no_argument, NULL, GAUGE},
	{"infobox",      no_argument, NULL, INFOBOX},
	{"inputbox",     no_argument, NULL, INPUTBOX},
	{"menu",         no_argument, NULL, MENU},
#if 0
	{"mixedform",    no_argument, NULL, MIXEDFORM},
#endif
	{"mixedgauge",   no_argument, NULL, MIXEDGAUGE},
	{"msgbox",       no_argument, NULL, MSGBOX},
	{"passwordbox",  no_argument, NULL, PASSWORDBOX},
	{"passwordform", no_argument, NULL, PASSWORDFORM},
	{"pause",        no_argument, NULL, PAUSE},
	{"radiolist",    no_argument, NULL, RADIOLIST},
	{"rangebox",     no_argument, NULL, RANGEBOX},
	{"textbox",      no_argument, NULL, TEXTBOX},
	{"timebox",      no_argument, NULL, TIMEBOX},
	{"treeview",     no_argument, NULL, TREEVIEW},
	{"yesno",        no_argument, NULL, YESNO},
	/* END */
	{ NULL, 0, NULL, 0}
};


/* prototypes */
static void _gbsddialog_backtitle(GBSDDialog * gbd);

static void _gbsddialog_clear_screen(GBSDDialog * gbd);

static int _gbsddialog_parseargs(GBSDDialog * gbd,
		int argc, char const ** argv);

static gboolean _gbsddialog_theme_load(GBSDDialog * gbd, char const * theme);
static gboolean _gbsddialog_theme_save(GBSDDialog * gbd, char const * filename);


/* public */
/* functions */
/* gbsddialog */
static gboolean _gbsddialog_on_idle(gpointer data);
static gboolean _gbsddialog_on_idle_quit(gpointer data);
#if GTK_CHECK_VERSION(2, 2, 0)
static void _backtitle_on_size_changed(gpointer data);
#endif

int gbsddialog(int * ret, int argc, char const ** argv)
{
	GBSDDialog * gbd;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, \"%s\")\n", __func__, argc, argv[0]);
#endif
	if((gbd = malloc(sizeof(*gbd))) == NULL)
		return -error(errno, "%s", strerror(errno));
	memset(gbd, 0, sizeof(*gbd));
	gbd->ret = ret;
	gbd->argc = argc;
	gbd->argv = argv;
	if((gbd->screen = gdk_screen_get_default()) == NULL)
	{
		error(BSDDIALOG_ERROR, "Could not get the default screen");
		free(gbd);
		return -1;
	}
	g_idle_add(_gbsddialog_on_idle, gbd);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => 0\n", __func__);
#endif
	return 0;
}

static gboolean _gbsddialog_on_idle(gpointer data)
{
	GBSDDialog * gbd = data;
	struct bsddialog_conf * conf = &gbd->conf;
	struct options * opt = &gbd->opt;
	int parsed, argc, oi = optind;
	char const ** argv;
	char * text = NULL;
	int rows = BSDDIALOG_AUTOSIZE, cols = BSDDIALOG_AUTOSIZE;
	int res;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(gbd->argc=%d gbd->argv=\"%s\")\n",
			__func__, gbd->argc,
			(gbd->argc > 0) ? gbd->argv[0] : "(null)");
#endif
	if((parsed = _gbsddialog_parseargs(gbd, gbd->argc, gbd->argv)) <= 0)
	{
		*gbd->ret = EXITCODE(BSDDIALOG_ERROR);
		return _gbsddialog_on_idle_quit(gbd);
	}
	argc = parsed - optind;
	argv = gbd->argv + optind;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() argc=%d (optind=%d, parsed=%d)\n",
			__func__, argc, optind, parsed);
#endif

	if(opt->savethemefile != NULL)
	{
		_gbsddialog_theme_save(gbd, opt->savethemefile);
		opt->savethemefile = NULL;
	}
	if(opt->mandatory_dialog && opt->dialogbuilder == NULL)
	{
		*gbd->ret = EXITCODE(error(BSDDIALOG_ERROR,
					"expected a --<dialog>"));
		return _gbsddialog_on_idle_quit(gbd);
	}
	if(opt->loadthemefile != NULL)
	{
		_gbsddialog_theme_load(gbd, opt->loadthemefile);
		opt->loadthemefile = NULL;
	}
	if(opt->clearscreen)
		_gbsddialog_clear_screen(gbd);
	if(opt->backtitle != NULL && gbd->windows == NULL)
		_gbsddialog_backtitle(gbd);
	if(opt->dialogbuilder != NULL)
	{
		if(argc < 3)
		{
			*gbd->ret = EXITCODE(error(BSDDIALOG_ERROR,
						"expected <text> <rows> <cols>"));
			return _gbsddialog_on_idle_quit(gbd);
		}
		if((text = strdup(argv[0])) == NULL)
		{
			*gbd->ret = EXITCODE(error(BSDDIALOG_ERROR,
						"cannot allocate <text>"));
			return _gbsddialog_on_idle_quit(gbd);
		}
		rows = (int)strtol(argv[1], NULL, 10);
		cols = (int)strtol(argv[2], NULL, 10);

		if(opt->dialogbuilder != builder_textbox)
			custom_text(opt, argv[0], text);

		/* FIXME implement conf->text.escape/highlight */

		res = opt->dialogbuilder(conf, text, rows, cols,
				argc - 3, argv + 3, opt);
		*gbd->ret = EXITCODE(res);
		free(text);
		if(res == BSDDIALOG_ERROR)
			return _gbsddialog_on_idle_quit(gbd);
		if(conf->get_height != NULL && conf->get_width != NULL)
			dprintf(opt->output_fd, "DialogSize: %d, %d\n",
					*conf->get_height, *conf->get_width);
		if(res == BSDDIALOG_CANCEL || res == BSDDIALOG_ESC)
			return _gbsddialog_on_idle_quit(gbd);
	}
	else
		/* FIXME report error */
		return _gbsddialog_on_idle_quit(gbd);

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => %d gbd->argc=%d gbd->argv=\"%s\"\n",
			__func__, *gbd->ret, gbd->argc,
			(gbd->argc > 0) ? gbd->argv[0] : "(null)");
#endif

	if(parsed == gbd->argc)
		return _gbsddialog_on_idle_quit(gbd);

	gbd->argv[parsed - 1] = gbd->argv[0];
	gbd->argv += parsed - 1;
	gbd->argc -= parsed - 1;
	optind = oi;

	return TRUE;
}

static gboolean _gbsddialog_on_idle_quit(gpointer data)
{
	GBSDDialog * gbd = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	gtk_main_quit();
	free(gbd);
	return FALSE;
}


/* private */
/* gbsddialog_backtitle */
#if GTK_CHECK_VERSION(3, 0, 0)
static void _backtitle_apply_style(GtkWidget * widget,
		GdkRGBA * bg, GdkRGBA * fg);
static void _backtitle_bikeshed_color(GdkRGBA * color);
#else
static void _backtitle_apply_style(GtkWidget * widget,
		GdkColor * bg, GdkColor * fg);
static void _backtitle_bikeshed_color(GdkColor * color);
#endif

static void _gbsddialog_backtitle(GBSDDialog * gbd)
{
#if GTK_CHECK_VERSION(2, 2, 0)
	g_signal_connect_swapped(gbd->screen, "size-changed",
			G_CALLBACK(_backtitle_on_size_changed), gbd);
#endif
	_backtitle_on_size_changed(gbd);
}

#if GTK_CHECK_VERSION(3, 0, 0)
static void _backtitle_apply_style(GtkWidget * widget, GdkRGBA * bg,
		GdkRGBA * fg)
#else
static void _backtitle_apply_style(GtkWidget * widget, GdkColor * bg,
		GdkColor * fg)
#endif
{
#if GTK_CHECK_VERSION(3, 0, 0)
	GdkRGBA color = { 0.0, 0.0, 0.0, 1.0 };
#else
	GdkColor color = { 0, 0, 0, 0 };
#endif

	if(bg == NULL)
	{
		bg = &color;
		_backtitle_bikeshed_color(bg);
	}
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_widget_override_background_color(widget, GTK_STATE_FLAG_NORMAL, bg);
#else
	gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, bg);
#endif
	if(fg == NULL)
	{
		fg = &color;
		_backtitle_bikeshed_color(fg);
	}
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_widget_override_color(widget, GTK_STATE_FLAG_NORMAL, fg);
#else
	gtk_widget_modify_fg(widget, GTK_STATE_NORMAL, fg);
#endif
}

#if GTK_CHECK_VERSION(3, 0, 0)
static void _backtitle_bikeshed_color(GdkRGBA * color)
{
	color->red = random();
	color->red /= RAND_MAX;
	color->green = random();
	color->green /= RAND_MAX;
	color->blue = random();
	color->blue /= RAND_MAX;
}
#else
static void _backtitle_bikeshed_color(GdkColor * color)
{
	color->red = random();
	color->green = random();
	color->blue = random();
}
#endif

#if GTK_CHECK_VERSION(2, 2, 0)
static void _backtitle_on_size_changed(gpointer data)
{
	GBSDDialog * gbd = data;
#if GTK_CHECK_VERSION(3, 22, 0)
	GdkDisplay * display;
	GdkMonitor * monitor;
#endif
	GdkRectangle geometry;
	gint scale = 1;
	size_t i;
	GtkWidget ** p;
	GtkWidget * window, * container, * separator, * widget;
#if GTK_CHECK_VERSION(3, 0, 0)
	GtkStyleContext * style;
#else
	GtkStyle * style;
#endif
#if GTK_CHECK_VERSION(3, 0, 0)
	GdkRGBA bg = { 0.0, 0.0, 0.0, 1.0 };
	GdkRGBA fg = { 0.0, 0.0, 0.0, 1.0 };
#else
	GdkColor bg = { 0, 0, 0, 65535 };
	GdkColor fg = { 0, 65535, 65535, 65535 };
#endif
	PangoFontDescription * fontdesc;
	char const * logo;

	/* XXX this will cause flickering */
	for(i = 0; i < gbd->windows_cnt; i++)
		gtk_widget_destroy(gbd->windows[i]);
	gbd->label = NULL;
#if GTK_CHECK_VERSION(3, 22, 0)
	display = gdk_screen_get_display(gbd->screen);
	i = gdk_display_get_n_monitors(display);
#else
	i = 1;
#endif
	if((p = realloc(gbd->windows, (sizeof(*gbd->windows) * i))) == NULL)
	{
		/* FIXME report the error */
		free(gbd->windows);
		gbd->windows_cnt = 0;
		return;
	}
	gbd->windows = p;
	gbd->windows_cnt = i;

	/* obtain background and foreground colors */
	if(gbd->opt.bikeshed)
	{
		_backtitle_bikeshed_color(&bg);
		_backtitle_bikeshed_color(&fg);
	}
	else
	{
		widget = gtk_tree_view_new();
#if GTK_CHECK_VERSION(3, 0, 0)
		style = gtk_widget_get_style_context(widget);
		gtk_style_context_get_background_color(style,
				GTK_STATE_FLAG_SELECTED, &bg);
		gtk_style_context_get_color(style, GTK_STATE_FLAG_SELECTED,
				&fg);
#else
		style = gtk_widget_get_style(widget);
		/* FIXME obtain the colors */
#endif
		gtk_widget_destroy(widget);
	}

	/* obtain the logo location if set */
	logo = getenv("GBSDDIALOG_BACKTITLE_LOGO");

	for(i = 0; i < gbd->windows_cnt; i++)
	{
		gbd->windows[i] = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		window = gbd->windows[i];
		_backtitle_apply_style(window, &bg, &fg);
#if GTK_CHECK_VERSION(3, 10, 0)
		scale = gtk_widget_get_scale_factor(window);
#endif
#if GTK_CHECK_VERSION(3, 22, 0)
		monitor = gdk_display_get_monitor(display, i);
		gdk_monitor_get_geometry(monitor, &geometry);
#else
		geometry.x = 0;
		geometry.y = 0;
		geometry.width = gdk_screen_get_width(gbd->screen);
		geometry.height = gdk_screen_get_height(gbd->screen);
#endif
		geometry.width = geometry.width * scale;
		geometry.height = geometry.height * scale;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() %dx%d\n", __func__,
				geometry.width, geometry.height);
#endif
		gtk_window_set_default_size(GTK_WINDOW(window),
				geometry.width, geometry.height);
		gtk_window_set_type_hint(GTK_WINDOW(window),
				GDK_WINDOW_TYPE_HINT_DESKTOP);
		gtk_window_move(GTK_WINDOW(window), geometry.x, geometry.y);
		gtk_window_resize(GTK_WINDOW(window),
				geometry.width * scale,
				geometry.height * scale);
#if GTK_CHECK_VERSION(3, 22, 0)
		if(gdk_monitor_is_primary(monitor))
		{
			gbd->label = gtk_label_new(gbd->opt.backtitle);
			widget = gbd->label;
		}
		else
#endif
			widget = gtk_label_new(" ");
		gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
#if GTK_CHECK_VERSION(3, 14, 0)
		gtk_widget_set_halign(widget, GTK_ALIGN_START);
#else
		gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
#endif
		fontdesc = pango_font_description_from_string(
				"Sans Bold Italic 32");
#if GTK_CHECK_VERSION(3, 0, 0)
		gtk_widget_override_font(widget, fontdesc);
#else
		gtk_widget_modify_font(widget, fontdesc);
#endif
		pango_font_description_free(fontdesc);
#if GTK_CHECK_VERSION(3, 0, 0)
		container = gtk_box_new(GTK_ORIENTATION_VERTICAL, BORDER_WIDTH);
#else
		container = gtk_vbox_new(FALSE, BORDER_WIDTH);
#endif
		gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);
		if(gbd->conf.no_lines != true)
		{
#if GTK_CHECK_VERSION(3, 0, 0)
			separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
			separator = gtk_hseparator_new();
#endif
			_backtitle_apply_style(separator, &fg, &fg);
			gtk_box_pack_start(GTK_BOX(container), separator, FALSE,
					TRUE, BORDER_WIDTH);
		}
		if(logo != NULL && access(logo, R_OK) == 0)
		{
			widget = gtk_image_new_from_file(logo);
#if GTK_CHECK_VERSION(3, 14, 0)
			gtk_widget_set_halign(widget, GTK_ALIGN_END);
#else
			gtk_misc_set_alignment(GTK_MISC(widget), 1.0, 0.5);
#endif
			gtk_box_pack_end(GTK_BOX(container), widget, FALSE,
					TRUE, 0);
		}
		gtk_container_add(GTK_CONTAINER(window), container);
		gtk_container_set_border_width(GTK_CONTAINER(window), 16);
		gtk_widget_show_all(window);
	}
}
#endif


/* gbsddialog_clear_screen */
static void _gbsddialog_clear_screen(GBSDDialog * gbd)
{
	GdkWindow * root;
	GtkWidget * widget;
#if GTK_CHECK_VERSION(3, 0, 0)
	GtkStyleContext * style;
	GdkRGBA color = { 0.0, 0.0, 0.0, 1.0 };
#else
	GtkStyle * style;
#endif

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	root = gdk_screen_get_root_window(gbd->screen);
	widget = gtk_tree_view_new();
#if GTK_CHECK_VERSION(3, 0, 0)
	style = gtk_widget_get_style_context(widget);
	gtk_style_context_get_background_color(style,
			GTK_STATE_FLAG_SELECTED, &color);
	gdk_window_set_background_rgba(root, &color);
#else
	style = gtk_widget_get_style(widget);
	/* FIXME this does not seem to work */
	gtk_style_set_background(style, root, GTK_STATE_NORMAL);
#endif
	gtk_widget_destroy(widget);
}


/* gbsddialog_parseargs */
static int _parseargs_arg(GBSDDialog * gbd, struct bsddialog_conf * conf,
		struct options * opt, int arg);

static int _gbsddialog_parseargs(GBSDDialog * gbd, int argc, char const ** argv)
{
	int ret;
	int arg, i;
	struct bsddialog_conf * conf = &gbd->conf;
	struct options * opt = &gbd->opt;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, argc);
#endif

	memset(conf, 0, sizeof(*conf));
	conf->y = BSDDIALOG_AUTOSIZE;
	conf->x = BSDDIALOG_AUTOSIZE;
	conf->shadow = true;
	conf->text.cols_per_row = DEFAULT_COLS_PER_ROW;
	conf->key.enable_esc = true;
	conf->button.always_active = true;

	memset(opt, 0, sizeof(*opt));
	opt->theme = -1;
	opt->output_fd = STDERR_FILENO;
	opt->max_input_form = 2048;
	opt->mandatory_dialog = true;

	for(i = 0; i < argc; i++)
		if(strcmp(argv[i], "--and-dialog") == 0
				|| strcmp(argv[i], "--and-widget") == 0)
		{
			argc = i + 1;
			break;
		}
	while((arg = getopt_long(argc, argv, "", longopts, NULL)) != -1)
		if((ret = _parseargs_arg(gbd, conf, opt, arg)) != 0)
			return ret;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => %d\n", __func__, argc);
#endif
	return argc;
}

static int _parseargs_arg(GBSDDialog * gbd, struct bsddialog_conf * conf,
		struct options * opt, int arg)
{
	GdkRectangle workarea;
	gdouble ex;

	switch(arg)
	{
		/* Options */
#ifdef WITH_XDIALOG
		case ALLOW_CLOSE:
			conf->key.enable_esc = true;
			break;
#endif
		case ALTERNATE_SCREEN:
		case ASCII_LINES:
		case NORMAL_SCREEN:
			/* no-op */
			break;
		case AND_DIALOG:
			if(opt->dialogbuilder == NULL)
				return -error(BSDDIALOG_ERROR,
						"--and-dialog without"
						" previous --<dialog>");
			break;
		case BACKTITLE:
			opt->backtitle = optarg;
			break;
		case BEGIN_X:
			conf->x = (int)strtol(optarg, NULL, 10);
			if(conf->x < BSDDIALOG_CENTER)
				return -error(BSDDIALOG_ERROR,
						"--begin-x %d is < %d",
						conf->x, BSDDIALOG_CENTER);
			break;
		case BEGIN_Y:
			conf->y = (int)strtol(optarg, NULL, 10);
			if(conf->y < BSDDIALOG_CENTER)
				return -error(BSDDIALOG_ERROR,
						"--begin-y %d is < %d",
						conf->y, BSDDIALOG_CENTER);
			conf->auto_topmargin = 0;
			break;
		case BIKESHED:
			opt->bikeshed = true;
			break;
		case CANCEL_EXIT_CODE:
			exitcodes[BSDDIALOG_CANCEL + 1].value
				= strtol(optarg, NULL, 10);
			break;
		case CANCEL_LABEL:
			conf->button.cancel_label = optarg;
			break;
		case CLEAR_DIALOG:
			conf->clear = true;
			break;
		case CLEAR_SCREEN:
			opt->mandatory_dialog = false;
			opt->clearscreen = true;
			break;
		case CR_WRAP:
			opt->cr_wrap = true;
			break;
		case DEFAULT_ITEM:
			opt->item_default = optarg;
			break;
		case DATE_FORMAT:
			opt->date_fmt = optarg;
			break;
		case DEFAULT_NO:
			conf->button.default_cancel = true;
			break;
		case DISABLE_ESC:
			conf->key.enable_esc = false;
			break;
#ifdef WITH_XDIALOG
		case EDITABLE:
			opt->editable = true;
			break;
#endif
		case ERROR_EXIT_CODE:
			exitcodes[BSDDIALOG_ERROR + 1].value
				= strtol(optarg, NULL, 10);
			break;
		case ESC_EXIT_CODE:
			exitcodes[BSDDIALOG_ESC + 1].value
				= strtol(optarg, NULL, 10);
			break;
		case EXIT_LABEL:
			conf->button.ok_label = optarg;
			break;
		case EXTRA_BUTTON:
			conf->button.with_extra = true;
			break;
		case EXTRA_EXIT_CODE:
			exitcodes[BSDDIALOG_EXTRA + 1].value
				= strtol(optarg, NULL, 10);
			break;
		case EXTRA_LABEL:
			conf->button.extra_label = optarg;
			break;
#ifdef WITH_XDIALOG
		case FIXED_FONT:
			opt->fixed_font = true;
			break;
		case HELP:
			opt->help = optarg;
			break;
#endif
		case HELP_BUTTON:
			conf->button.with_help = true;
			break;
		case HELP_EXIT_CODE:
			exitcodes[BSDDIALOG_HELP + 1].value
				= strtol(optarg, NULL, 10);
			break;
		case HELP_LABEL:
			conf->button.help_label = optarg;
			break;
		case HFILE:
			conf->key.f1_file = optarg;
			break;
		case HLINE:
			if(optarg[0] != '\0')
				conf->bottomtitle = optarg;
			break;
		case HMSG:
			conf->key.f1_message = optarg;
			break;
#ifdef WITH_XDIALOG
		case ICON:
			opt->icon = optarg;
			break;
#endif
		case IGNORE:
			opt->ignore = true;
			break;
#ifdef WITH_XDIALOG
		case IGNORE_EOF:
			opt->ignore_eof = true;
			break;
#endif
		case INSECURE:
			conf->form.securech = '*';
			break;
		case ITEM_BOTTOM_DESC:
			opt->item_bottomdesc = true;
			break;
		case ITEM_DEPTH:
			opt->item_depth = true;
			break;
		case ITEM_PREFIX:
			opt->item_prefix = true;
			break;
		case LOAD_THEME:
			opt->loadthemefile = optarg;
			break;
		case MAX_INPUT:
			opt->max_input_form = strtoul(optarg, NULL, 10);
			break;
#ifdef WITH_XDIALOG
		case NO_BUTTONS:
			/* XXX should be in struct bsddialog_conf */
			opt->without_buttons = true;
			break;
#endif
		case NO_CANCEL:
			conf->button.without_cancel = true;
			break;
#ifdef WITH_XDIALOG
		case NO_CR_WRAP:
			opt->cr_wrap = false;
			break;
#endif
		case NO_DESCRIPTIONS:
			conf->menu.no_desc = true;
			break;
		case NO_LINES:
			conf->no_lines = true;
			break;
		case NO_NAMES:
			conf->menu.no_name = true;
			break;
		case NO_OK:
			conf->button.without_ok = true;
			break;
		case NO_SHADOW:
			conf->shadow = false;
			break;
		case OK_EXIT_CODE:
			exitcodes[BSDDIALOG_OK + 1].value
				= strtol(optarg, NULL, 10);
			break;
		case OK_LABEL:
			conf->button.ok_label = optarg;
			break;
		case OUTPUT_FD:
			opt->output_fd = strtol(optarg, NULL, 10);
			break;
		case OUTPUT_SEPARATOR:
			opt->item_output_sep = optarg;
			break;
		case PRINT_MAXSIZE:
			opt->mandatory_dialog = false;
			get_workarea(gbd->screen, &workarea);
			ex = get_font_size(gbd->screen);
			dprintf(opt->output_fd, "MaxSize: %d, %d\n",
					(int)(workarea.height / ex / 2),
					(int)(workarea.width / ex));
			break;
		case PRINT_SIZE:
			conf->get_height = &opt->getH;
			conf->get_width = &opt->getW;
			break;
		case PRINT_VERSION:
			opt->mandatory_dialog = false;
			dprintf(opt->output_fd,
					"Version: %s (libbsddialog: %s)\n",
					VERSION, LIBBSDDIALOG_VERSION);
			break;
		case QUOTED:
			opt->item_always_quote = true;
			break;
		case SAVE_THEME:
			opt->mandatory_dialog = false;
			opt->savethemefile = optarg;
			break;
		case SEPARATE_OUTPUT:
			opt->item_output_sepnl = true;
			break;
		case SHADOW:
			conf->shadow = true;
			break;
		case SINGLE_QUOTED:
			opt->item_singlequote = true;
			break;
		case SLEEP:
			conf->sleep = strtoul(optarg, NULL, 10) * 1000;
			break;
		case STDERR:
			opt->output_fd = STDERR_FILENO;
			break;
		case STDOUT:
			opt->output_fd = STDOUT_FILENO;
			break;
		case SWITCH_BUTTONS:
			conf->button.always_active = false;
			break;
		case TAB_ESCAPE:
			opt->tab_escape = true;
			break;
		case TEXT_ESCAPE:
#ifdef __FreeBSD__
			/* XXX ugly compatibility fix for now */
			conf->text.highlight = true;
#else
			conf->text.escape = true;
#endif
			break;
		case TEXT_UNCHANGED:
			opt->text_unchanged = true;
			break;
		case THEME:
			g_object_set(gtk_settings_get_default(),
					"gtk-theme-name", optarg, NULL);
			break;
		case TIMEOUT_EXIT_CODE:
			exitcodes[BSDDIALOG_TIMEOUT + 1].value
				= strtol(optarg, NULL, 10);
			break;
		case TIME_FORMAT:
			opt->time_fmt = optarg;
			break;
		case TITLE:
			conf->title = optarg;
			break;
#ifdef WITH_XDIALOG
		case WIZARD:
			opt->wizard = true;
			break;
#endif
		/* Dialogs */
		case CALENDAR:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --calendar without "
						"--and-dialog", opt->name);
			opt->name = "--calendar";
			opt->dialogbuilder = builder_calendar;
			break;
		case CHECKLIST:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --checklist without "
						"--and-dialog", opt->name);
			opt->name = "--checklist";
			opt->dialogbuilder = builder_checklist;
			conf->auto_downmargin = 1;
			break;
#ifdef WITH_XDIALOG
		case COLORSEL:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --colorsel without "
						"--and-dialog", opt->name);
			opt->name = "--colorsel";
			opt->dialogbuilder = builder_colorsel;
			conf->auto_downmargin = 1;
			break;
		case COMBOBOX:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --combobox without "
						"--and-dialog", opt->name);
			opt->name = "--combobox";
			opt->dialogbuilder = builder_combobox;
			conf->auto_downmargin = 1;
			break;
#endif
		case DATEBOX:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --datebox without "
						"--and-dialog", opt->name);
			opt->name = "--datebox";
			opt->dialogbuilder = builder_datebox;
			break;
#ifdef WITH_XDIALOG
		case DSELECT:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --dselect without "
						"--and-dialog", opt->name);
			opt->name = "--dselect";
			opt->dialogbuilder = builder_dselect;
			conf->auto_downmargin = 1;
			break;
		case FONTSEL:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --fontsel without "
						"--and-dialog", opt->name);
			opt->name = "--fontsel";
			opt->dialogbuilder = builder_fontsel;
			conf->auto_downmargin = 1;
			break;
#endif
		case FORM:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --form without "
						"--and-dialog", opt->name);
			opt->name = "--form";
			opt->dialogbuilder = builder_form;
			conf->auto_downmargin = 1;
			break;
#ifdef WITH_XDIALOG
		case FSELECT:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --fselect without "
						"--and-dialog", opt->name);
			opt->name = "--fselect";
			opt->dialogbuilder = builder_fselect;
			conf->auto_downmargin = 1;
			break;
#endif
		case GAUGE:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --gauge without "
						"--and-dialog", opt->name);
			opt->name = "--gauge";
			opt->dialogbuilder = builder_gauge;
			break;
		case INFOBOX:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --infobox without "
						"--and-dialog", opt->name);
			opt->name = "--infobox";
			opt->dialogbuilder = builder_infobox;
			break;
		case INPUTBOX:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --inputbox without "
						"--and-dialog", opt->name);
			opt->name = "--inputbox";
			opt->dialogbuilder = builder_inputbox;
			conf->auto_downmargin = 1;
			break;
#ifdef WITH_XDIALOG
		case INPUTSBOX2:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --2inputsbox without "
						"--and-dialog", opt->name);
			opt->name = "--2inputsbox";
			opt->dialogbuilder = builder_2inputsbox;
			conf->auto_downmargin = 1;
			break;
		case INPUTSBOX3:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --3inputsbox without "
						"--and-dialog", opt->name);
			opt->name = "--3inputsbox";
			opt->dialogbuilder = builder_3inputsbox;
			conf->auto_downmargin = 1;
			break;
#endif
		case MENU:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --menu without "
						"--and-dialog", opt->name);
			opt->name = "--menu";
			opt->dialogbuilder = builder_menu;
			conf->auto_downmargin = 1;
			break;
		case MIXEDGAUGE:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --mixedgauge without "
						"--and-dialog", opt->name);
			opt->name = "--mixedgauge";
			opt->dialogbuilder = builder_mixedgauge;
			break;
		case MSGBOX:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --msgbox without "
						"--and-dialog", opt->name);
			opt->name = "--msgbox";
			opt->dialogbuilder = builder_msgbox;
			break;
		case PASSWORDBOX:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --passwordbox without "
						"--and-dialog", opt->name);
			opt->name = "--passwordbox";
			opt->dialogbuilder = builder_passwordbox;
			conf->auto_downmargin = 1;
			break;
		case PASSWORDFORM:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --passwordform without "
						"--and-dialog", opt->name);
			opt->name = "--passwordform";
			opt->dialogbuilder = builder_passwordform;
			conf->auto_downmargin = 1;
			break;
		case PAUSE:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --pause without "
						"--and-dialog", opt->name);
			opt->name = "--pause";
			opt->dialogbuilder = builder_pause;
			break;
		case RADIOLIST:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --radiolist without "
						"--and-dialog", opt->name);
			opt->name = "--radiolist";
			opt->dialogbuilder = builder_radiolist;
			conf->auto_downmargin = 1;
			break;
		case RANGEBOX:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --rangebox without "
						"--and-dialog", opt->name);
			opt->name = "--rangebox";
			opt->dialogbuilder = builder_rangebox;
			break;
#ifdef WITH_XDIALOG
		case RANGESBOX2:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --2rangesbox without "
						"--and-dialog", opt->name);
			opt->name = "--2rangesbox";
			opt->dialogbuilder = builder_2rangesbox;
			break;
		case RANGESBOX3:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --3rangesbox without "
						"--and-dialog", opt->name);
			opt->name = "--3rangesbox";
			opt->dialogbuilder = builder_3rangesbox;
			break;
#endif
		case TEXTBOX:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --textbox without "
						"--and-dialog", opt->name);
			opt->name = "--textbox";
			opt->dialogbuilder = builder_textbox;
			break;
		case TIMEBOX:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --timebox without "
						"--and-dialog", opt->name);
			opt->name = "--timebox";
			opt->dialogbuilder = builder_timebox;
			break;
		case TREEVIEW:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --treeview without "
						"--and-dialog", opt->name);
			opt->name = "--treeview";
			opt->dialogbuilder = builder_treeview;
			conf->auto_downmargin = 1;
			break;
		case YESNO:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --yesno without "
						"--and-dialog", opt->name);
			opt->name = "--yesno";
			opt->dialogbuilder = builder_yesno;
			break;
		default: /* Error */
			if(opt->ignore == true)
				break;
			return -error(BSDDIALOG_ERROR, "--ignore to continue");
	}
	return 0;
}


/* gbsddialog_theme_load */
static gboolean _gbsddialog_theme_load(GBSDDialog * gbd, char const * theme)
{
#if GTK_CHECK_VERSION(3, 0, 0)
	GtkCssProvider * css;
	GError * e = NULL;

	css = gtk_css_provider_new();
	if(gtk_css_provider_load_from_path(css, theme, &e) != TRUE)
	{
		error(BSDDIALOG_ERROR, e->message);
		g_error_free(e);
		return FALSE;
	}
	gtk_style_context_add_provider_for_screen(gbd->screen,
			GTK_STYLE_PROVIDER(css),
			GTK_STYLE_PROVIDER_PRIORITY_USER);
	return TRUE;
#else
	(void) gbd;

	gtk_rc_parse(theme);
	return TRUE;
#endif
}


/* gbsddialog_theme_save */
static gboolean _gbsddialog_theme_save(GBSDDialog * gbd, char const * filename)
{
#if GTK_CHECK_VERSION(3, 0, 0)
	GtkCssProvider * css;
	FILE * fp;
	char * p;
	(void) gbd;

	css = gtk_css_provider_get_default();
	if((fp = fopen(filename, "w")) == NULL)
	{
		error(BSDDIALOG_ERROR, "%s: %s", filename, strerror(errno));
		return FALSE;
	}
	p = gtk_css_provider_to_string(css);
	fwrite(p, sizeof(*p), strlen(p), fp);
	free(p);
	if(fclose(fp) != 0)
	{
		error(BSDDIALOG_ERROR, "%s: %s", filename, strerror(errno));
		return FALSE;
	}
	return TRUE;
#else
	gboolean ret = TRUE;
	gchar ** f;
	FILE * fp1;
	FILE * fp2;
	char buf[BUFSIZ];
	size_t n;
	(void) gbd;

	if((fp1 = fopen(filename, "w")) == NULL)
	{
		error(BSDDIALOG_ERROR, "%s: %s", filename, strerror(errno));
		return FALSE;
	}
	for(f = gtk_rc_get_default_files(); f != NULL && *f != NULL; f++)
	{
		if((fp2 = fopen(*f, "r")) == NULL)
		{
			error(BSDDIALOG_ERROR, "%s: %s", *f, strerror(errno));
			ret = FALSE;
			continue;
		}
		while((n = fread(buf, sizeof(*buf), sizeof(buf), fp2)) > 0)
			if(fwrite(buf, sizeof(*buf), n, fp1) != n)
			{
				error(BSDDIALOG_ERROR, "%s: %s", *f,
						strerror(errno));
				ret = FALSE;
			}
		if(fclose(fp2) != 0)
		{
			error(BSDDIALOG_ERROR, "%s: %s", *f, strerror(errno));
			ret = FALSE;
		}
	}
	if(fclose(fp1) != 0)
	{
		error(BSDDIALOG_ERROR, "%s: %s", filename, strerror(errno));
		return FALSE;
	}
	return ret;
#endif
}
