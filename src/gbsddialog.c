/* gbsddialog */
/* gbsddialog.c */
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



#include <unistd.h>
#include <stdlib.h>
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


/* getopt_long */
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
	ERROR_EXIT_CODE,
	ESC_EXIT_CODE,
	EXIT_LABEL,
	EXTRA_BUTTON,
	EXTRA_EXIT_CODE,
	EXTRA_LABEL,
#ifdef WITH_XDIALOG
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
	/* Dialogs */
	CALENDAR,
	CHECKLIST,
	DATEBOX,
	FORM,
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
	TEXTBOX,
	TIMEBOX,
	TREEVIEW,
	YESNO
};

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
	{"error-exit-code",   required_argument, NULL, ERROR_EXIT_CODE},
	{"esc-exit-code",     required_argument, NULL, ESC_EXIT_CODE},
	{"exit-label",        required_argument, NULL, EXIT_LABEL},
	{"extra-button",      no_argument,       NULL, EXTRA_BUTTON},
	{"extra-exit-code",   required_argument, NULL, EXTRA_EXIT_CODE},
	{"extra-label",       required_argument, NULL, EXTRA_LABEL},
#ifdef WITH_XDIALOG
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
	{"load-theme",        required_argument, NULL, LOAD_THEME},
#endif
	{"max-input",         required_argument, NULL, MAX_INPUT},
#ifdef WITH_XDIALOG
	{"no-buttons",        no_argument,       NULL, NO_BUTTONS},
#endif
	{"no-cancel",         no_argument,       NULL, NO_CANCEL},
	{"nocancel",          no_argument,       NULL, NO_CANCEL},
#ifdef WITH_XDIALOG
	{"no-close",	      no_argument,	 NULL, DISABLE_ESC},
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
	{"save-theme",        required_argument, NULL, SAVE_THEME},
#endif
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
	{"yes-label",         required_argument, NULL, OK_LABEL},
	/* Dialogs */
#ifdef WITH_XDIALOG
	{ "2inputsbox",  no_argument, NULL, INPUTSBOX2},
	{ "3inputsbox",  no_argument, NULL, INPUTSBOX3},
#endif
	{"calendar",     no_argument, NULL, CALENDAR},
	{"checklist",    no_argument, NULL, CHECKLIST},
	{"datebox",      no_argument, NULL, DATEBOX},
	{"form",         no_argument, NULL, FORM},
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


/* types */
typedef struct _GBSDDialog
{
	int * ret;
	int argc;
	char const ** argv;
	GtkWidget * window;	/* for backtitle */
	GtkWidget * label;
} GBSDDialog;

/* prototypes */
static int _parseargs(int argc, char const ** argv,
		struct bsddialog_conf * conf, struct options * opt);


/* gbsddialog */
static void _gbsddialog_backtitle(GBSDDialog * gbd,
		struct bsddialog_conf const * conf, struct options * opt);
#if GTK_CHECK_VERSION(3, 0, 0)
static void _backtitle_apply_style(GtkWidget * widget,
		GdkRGBA * bg, GdkRGBA * fg);
static void _backtitle_bikeshed_color(GdkRGBA * color);
#else
static void _backtitle_apply_style(GtkWidget * widget,
		GdkColor * bg, GdkColor * fg);
static void _backtitle_bikeshed_color(GdkColor * color);
#endif
static gboolean _gbsddialog_on_idle(gpointer data);
static gboolean _gbsddialog_on_idle_quit(gpointer data);
#if GTK_CHECK_VERSION(2, 2, 0)
static void _backtitle_on_size_changed(GdkScreen * screen, gpointer data);
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
	g_idle_add(_gbsddialog_on_idle, gbd);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => 0\n", __func__);
#endif
	return 0;
}

static void _gbsddialog_backtitle(GBSDDialog * gbd,
		struct bsddialog_conf const * conf, struct options * opt)
{
	GdkScreen * screen;
	GtkWidget * widget;
	GtkWidget * separator;
#if GTK_CHECK_VERSION(3, 0, 0)
	GtkStyleContext * style;
#else
	GtkStyle * style;
#endif
	gint scale = 1;
	PangoFontDescription * fontdesc;
#if GTK_CHECK_VERSION(3, 0, 0)
	GdkRGBA bg = { 0.0, 0.0, 0.0, 1.0 };
	GdkRGBA fg = { 0.0, 0.0, 0.0, 1.0 };
#else
	GdkColor bg = { 0, 0, 0, 65535 };
	GdkColor fg = { 0, 65535, 65535, 65535 };
#endif

	if(gbd->label != NULL)
	{
		gtk_label_set_text(GTK_LABEL(gbd->label), opt->backtitle);
		return;
	}
	if((screen = gdk_screen_get_default()) == NULL)
		return;
	if(opt->bikeshed)
	{
		srandom(time(NULL) ^ getpid() ^ getuid());
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
	gbd->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	_backtitle_apply_style(gbd->window, &bg, &fg);
	/* FIXME:
	 * - draw a desktop window on each monitor instead? */
#if GTK_CHECK_VERSION(3, 10, 0)
	scale = gtk_widget_get_scale_factor(gbd->window);
#endif
	gtk_window_set_default_size(GTK_WINDOW(gbd->window),
			gdk_screen_get_width(screen) * scale,
			gdk_screen_get_height(screen) * scale);
#if GTK_CHECK_VERSION(2, 2, 0)
	g_signal_connect(screen, "size-changed",
			G_CALLBACK(_backtitle_on_size_changed), gbd);
#endif
	gtk_window_set_type_hint(GTK_WINDOW(gbd->window),
			GDK_WINDOW_TYPE_HINT_DESKTOP);
	gbd->label = gtk_label_new(opt->backtitle);
	gtk_label_set_justify(GTK_LABEL(gbd->label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment(GTK_MISC(gbd->label), 0.0, 0.5);
	fontdesc = pango_font_description_from_string("Sans Bold Italic 32");
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_widget_override_font(gbd->label, fontdesc);
#else
	gtk_widget_modify_font(gbd->label, fontdesc);
#endif
	pango_font_description_free(fontdesc);
#if GTK_CHECK_VERSION(3, 0, 0)
	widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
#else
	widget = gtk_vbox_new(FALSE, 4);
#endif
	gtk_box_pack_start(GTK_BOX(widget), gbd->label, FALSE, TRUE, 0);
	if(conf->no_lines != true)
	{
#if GTK_CHECK_VERSION(3, 0, 0)
		separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#else
		separator = gtk_hseparator_new();
#endif
		_backtitle_apply_style(separator, &fg, &fg);
		gtk_box_pack_start(GTK_BOX(widget), separator, FALSE, TRUE, 4);
	}
	gtk_container_add(GTK_CONTAINER(gbd->window), widget);
	gtk_container_set_border_width(GTK_CONTAINER(gbd->window), 16);
	gtk_widget_show_all(gbd->window);
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

static gboolean _gbsddialog_on_idle(gpointer data)
{
	GBSDDialog * gbd = data;
	int parsed, argc, oi = optind;
	char const ** argv;
	struct bsddialog_conf conf;
	struct options opt;
	char * text = NULL;
	int rows = BSDDIALOG_AUTOSIZE, cols = BSDDIALOG_AUTOSIZE;
	int res;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(gbd->argc=%d gbd->argv=\"%s\")\n",
			__func__, gbd->argc,
			(gbd->argc > 0) ? gbd->argv[0] : "(null)");
#endif
	if((parsed = _parseargs(gbd->argc, gbd->argv, &conf, &opt)) <= 0)
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

	if(opt.mandatory_dialog && opt.dialogbuilder == NULL)
	{
		*gbd->ret = EXITCODE(error(BSDDIALOG_ERROR,
					"expected a --<dialog>"));
		return _gbsddialog_on_idle_quit(gbd);
	}
	if(opt.backtitle != NULL && gbd->window == NULL)
		_gbsddialog_backtitle(gbd, &conf, &opt);
	if(opt.dialogbuilder != NULL)
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
		rows = (int)strtol(gbd->argv[1], NULL, 10);
		cols = (int)strtol(gbd->argv[2], NULL, 10);

		if(opt.dialogbuilder != builder_textbox)
			custom_text(&opt, argv[0], text);

		/* FIXME implement conf->text.escape/highlight */

		res = opt.dialogbuilder(&conf, text, rows, cols,
				argc - 3, argv + 3, &opt);
		*gbd->ret = EXITCODE(res);
		free(text);
		if(res == BSDDIALOG_ERROR)
			return _gbsddialog_on_idle_quit(gbd);
		if(conf.get_height != NULL && conf.get_width != NULL)
			dprintf(opt.output_fd, "DialogSize: %d, %d\n",
					*conf.get_height, *conf.get_width);
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

#if GTK_CHECK_VERSION(2, 2, 0)
static void _backtitle_on_size_changed(GdkScreen * screen, gpointer data)
{
	GBSDDialog * gbd = data;
	gint scale = 1;

#if GTK_CHECK_VERSION(3, 10, 0)
	scale = gtk_widget_get_scale_factor(gbd->window);
#endif
	gtk_window_resize(GTK_WINDOW(gbd->window),
			gdk_screen_get_width(screen) * scale,
			gdk_screen_get_height(screen) * scale);
}
#endif


/* parseargs */
static int _parsearg(struct bsddialog_conf * conf, struct options * opt,
		int arg);

static int _parseargs(int argc, char const ** argv,
		struct bsddialog_conf * conf, struct options * opt)
{
	int ret;
	int arg, i;

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
		if((ret = _parsearg(conf, opt, arg)) != 0)
			return ret;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => %d\n", __func__, argc);
#endif
	return argc;
}

static int _parsearg(struct bsddialog_conf * conf, struct options * opt,
		int arg)
{
	GdkScreen * screen;
	GtkStyleContext * style;
	gdouble ex, fontsize, resolution;
	GdkRectangle workarea;
	PangoFontDescription const * fontdesc;

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
			/* obtain the default screen */
			if((screen = gdk_screen_get_default()) == NULL)
				return -error(BSDDIALOG_ERROR, "Could not"
						" obtain the screen size");
			/* obtain the workarea */
			gdk_screen_get_monitor_workarea(screen,
					gdk_screen_get_primary_monitor(screen),
					&workarea);
			/* obtain the default font size */
			style = gtk_style_context_new();
			fontdesc = gtk_style_context_get_font(style,
					GTK_STATE_FLAG_NORMAL);
			fontsize = pango_font_description_get_size(fontdesc);
			if(pango_font_description_get_size_is_absolute(
						fontdesc))
				ex = fontsize;
			else
			{
				resolution = gdk_screen_get_resolution(screen);
				ex = (fontsize * resolution)
					/ (72.0 * PANGO_SCALE);
			}
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
		case DATEBOX:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --datebox without "
						"--and-dialog", opt->name);
			opt->name = "--datebox";
			opt->dialogbuilder = builder_datebox;
			break;
		case FORM:
			if(opt->dialogbuilder != NULL)
				return -error(BSDDIALOG_ERROR,
						"%s and --form without "
						"--and-dialog", opt->name);
			opt->name = "--form";
			opt->dialogbuilder = builder_form;
			conf->auto_downmargin = 1;
			break;
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
