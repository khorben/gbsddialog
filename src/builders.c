/* gbsddialog */
/* builders.c */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
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



#include <stdio.h>
#include <strings.h>
#include <time.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "callbacks.h"
#include "builders.h"

#ifndef MIN
# define MIN(a, b) ((a) <= (b) ? (a) : (b))
#endif


/* builders */
/* types */
struct datebox_data
{
	GtkWidget * day;
	GtkWidget * month;
	GtkWidget * year;
};

struct gauge_data
{
	GtkWidget * dialog;
	GtkWidget * label;
	GtkWidget * widget;	/* progress bar */
	guint id;
	int sep;		/* -1 no, 0 yes, 1 percentage */
};

struct infobox_data
{
	GtkWidget * dialog;
	guint id;
};

struct pause_data
{
	GtkWidget * dialog;
	GtkWidget * widget;	/* progress bar */
	unsigned int secs;
	gdouble step;
	guint id;
};

struct timebox_data
{
	GtkWidget * hour;
	GtkWidget * minute;
	GtkWidget * second;
	guint id;
};


/* prototypes */
static GtkWidget * _builder_dialog(struct bsddialog_conf const * conf,
		char const * text, int rows);
static void _builder_dialog_buttons(GtkWidget * dialog,
		struct bsddialog_conf const * conf);
static int _builder_dialog_error(GtkWidget * parent,
		struct bsddialog_conf const * conf, char const * error);
static int _builder_dialog_help(GtkWidget * parent,
		struct bsddialog_conf const * conf);
static int _builder_dialog_output(struct bsddialog_conf const * conf,
		struct options const * opt, int res);
static int _builder_dialog_run(GtkWidget * dialog);


/* functions */
/* builder_calendar */
static void _calendar_on_day_activated(gpointer data);

int builder_calendar(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * widget;
	guint year, month, day;
	struct tm tm;
	char buf[1024];
	char const * fmt = "%d/%m/%Y";
	size_t len;

	if(argc == 3)
	{
		day = strtoul(argv[0], NULL, 10);
		month = strtoul(argv[1], NULL, 10);
		year = strtoul(argv[2], NULL, 10);
	}
	else if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, text, rows);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	widget = gtk_calendar_new();
	if(argc == 3 && day <= 31 && month >= 1 && month <= 12 && year != 0)
	{
		gtk_calendar_select_day(GTK_CALENDAR(widget), day);
		gtk_calendar_select_month(GTK_CALENDAR(widget), month - 1,
				year);
	}
	g_signal_connect_swapped(widget, "day-selected-double-click",
			G_CALLBACK(_calendar_on_day_activated), dialog);
	gtk_box_pack_start(GTK_BOX(container), widget, TRUE, TRUE, 4);
	gtk_widget_show(widget);
	_builder_dialog_buttons(dialog, conf);
	ret = _builder_dialog_run(dialog);
	gtk_calendar_get_date(GTK_CALENDAR(widget), &year, &month, &day);
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			if(opt->date_fmt != NULL)
				fmt = opt->date_fmt;
			memset(&tm, 0, sizeof(tm));
			tm.tm_mday = day;
			tm.tm_mon = month;
			tm.tm_year = year - 1900;
			len = strftime(buf, sizeof(buf) - 1, fmt, &tm);
			buf[len] = '\n';
			write(opt->output_fd, buf, len + 1);
			return ret;
	}
	return _builder_dialog_output(conf, opt, ret);
}

static void _calendar_on_day_activated(gpointer data)
{
	GtkWidget * dialog = data;

	gtk_window_activate_default(GTK_WINDOW(dialog));
}


/* builder_checklist */
static void _checklist_on_row_activated(GtkWidget * widget, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data);
static void _checklist_on_row_toggled(GtkCellRenderer * renderer, char * path,
		gpointer data);

int builder_checklist(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * window;
	GtkWidget * widget;
	GtkListStore * store;
	GtkTreeIter iter;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	GtkTreeSelection * treesel;
	int i, j, n;
	gboolean b, set;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %d, %d (%d), \"%s\")\n", __func__, rows, cols,
			argc, (argc - 1) / 3,
			(argv[0] != NULL) ? argv[0] : "(null)");
#endif
	j = opt->item_bottomdesc ? 4 : 3;
	if(argc < 1)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	if((n = strtol(argv[0], NULL, 10)) > (argc - 1) / j)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	else if(n == 0)
		n = (argc - 1) / j;
	dialog = _builder_dialog(conf, text, rows);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	store = gtk_list_store_new(j, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	widget = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(widget), GTK_TREE_MODEL(store));
	if(opt->item_bottomdesc)
		gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(widget), 3);
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_BROWSE);
	for(i = 0; i < n; i++)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				0, strcasecmp(argv[i * j + 3], "on") == 0,
				1, argv[i * j + 1], 2, argv[i * j + 2],
				(j == 4) ? 3 : -1,
				(j == 4) ? argv[i * j + 4] : NULL, -1);
		if(opt->item_default != NULL && strcmp(argv[i * j + 1],
					opt->item_default) == 0)
			gtk_tree_selection_select_iter(treesel, &iter);
	}
	renderer = gtk_cell_renderer_toggle_new();
	g_signal_connect(renderer, "toggled",
			G_CALLBACK(_checklist_on_row_toggled), store);
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
			"active", 0, NULL);
	gtk_tree_view_column_set_expand(column, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	if(conf->menu.no_name == false)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(), "text", 1, NULL);
		gtk_tree_view_column_set_expand(column, FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	if(conf->menu.no_desc == false)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(), "text", 2, NULL);
		gtk_tree_view_column_set_expand(column, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	g_signal_connect(widget, "row-activated",
			G_CALLBACK(_checklist_on_row_activated), NULL);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 0);
	gtk_widget_show_all(window);
	_builder_dialog_buttons(dialog, conf);
	ret = _builder_dialog_run(dialog);
	switch(ret)
	{
		case BSDDIALOG_HELP:
			if(gtk_tree_selection_get_selected(treesel, NULL, &iter)
					== FALSE)
				break;
			gtk_tree_model_get(GTK_TREE_MODEL(store), &iter,
					1, &p, -1);
			dprintf(opt->output_fd, "HELP %s\n", p);
			free(p);
			break;
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			for(b = gtk_tree_model_get_iter_first(
						GTK_TREE_MODEL(store), &iter);
					b != FALSE;
					b = gtk_tree_model_iter_next(
						GTK_TREE_MODEL(store), &iter))
			{
				gtk_tree_model_get(GTK_TREE_MODEL(store), &iter,
						0, &set, 1, &p, -1);
				if(set)
					dprintf(opt->output_fd, "%s\n", p);
				free(p);
			}
			break;
		default:
			_builder_dialog_output(conf, opt, ret);
			break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}

static void _checklist_on_row_activated(GtkWidget * widget, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data)
{
	GtkTreeModel * model;
	GtkTreeIter iter;
	gboolean set;
	(void) column;
	(void) data;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	if(gtk_tree_model_get_iter(model, &iter, path) == FALSE)
		return;
	gtk_tree_model_get(model, &iter, 0, &set, -1);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, set ? FALSE : TRUE,
			-1);
}

static void _checklist_on_row_toggled(GtkCellRenderer * renderer, char * path,
		gpointer data)
{
	GtkListStore * store = data;
	GtkTreePath * tp;
	GtkTreeIter iter;
	gboolean b;

	if((tp = gtk_tree_path_new_from_string(path)) == NULL)
		return;
	b = gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, tp);
	gtk_tree_path_free(tp);
	if(b == FALSE)
		return;
	gtk_list_store_set(store, &iter, 0, gtk_cell_renderer_toggle_get_active(
				GTK_CELL_RENDERER_TOGGLE(renderer))
			? FALSE : TRUE, -1);
}


/* builder_datebox */
static void _datebox_on_year_value_changed(GtkWidget * widget);

int builder_datebox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	struct datebox_data dd = { NULL, NULL, NULL };
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * box;
	GtkListStore * months;
	GtkCellRenderer * renderer;
	GtkTreeIter iter;
	guint i, year, month, day;
	time_t t;
	struct tm tm;
	char buf[1024];
	char const * fmt = "%d/%m/%Y";
	size_t len;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, argc);
#endif
	if(argc == 3)
	{
		day = strtoul(argv[0], NULL, 10);
		month = strtoul(argv[1], NULL, 10);
		year = strtoul(argv[2], NULL, 10);
	}
	else if(argc > 0)
	{
		error_args(opt->name, argc - 1, argv + 1);
		return BSDDIALOG_ERROR;
	}
	else if((t = time(NULL)) != (time_t)-1 && localtime_r(&t, &tm) == NULL)
	{
		/* FIXME report error */
		return BSDDIALOG_ERROR;
	}
	else
	{
		day = tm.tm_mday;
		month = tm.tm_mon + 1;
		year = tm.tm_year + 1900;
	}
	dialog = _builder_dialog(conf, text, rows);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_pack_start(GTK_BOX(box),
		       	gtk_label_new("Day: "), FALSE, TRUE, 0);
	dd.day = gtk_spin_button_new_with_range(1.0, 31.0, 1.0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(dd.day), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dd.day), (gdouble)day);
	gtk_box_pack_start(GTK_BOX(box), dd.day, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box),
			gtk_label_new("Month: "), FALSE, TRUE, 0);
	months = gtk_list_store_new(2, G_TYPE_LONG, G_TYPE_STRING);
	for(i = 0; i < 12; i++)
	{
		tm.tm_mon = i;
		strftime(buf, sizeof(buf) - 1, "%B", &tm);
		gtk_list_store_append(months, &iter);
		gtk_list_store_set(months, &iter, 0, i, 1, buf, -1);
	}
	dd.month = gtk_combo_box_new_with_model(GTK_TREE_MODEL(months));
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(dd.month), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(dd.month), renderer,
			"text", 1, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(dd.month), month - 1);
	gtk_box_pack_start(GTK_BOX(box), dd.month, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box),
			gtk_label_new("Year: "), FALSE, TRUE, 0);
	dd.year = gtk_spin_button_new_with_range(-9999.0, 9999.0, 1.0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(dd.year), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dd.year), (gdouble)year);
	g_signal_connect(dd.year, "value-changed",
		       	G_CALLBACK(_datebox_on_year_value_changed), NULL);
	gtk_box_pack_start(GTK_BOX(box), dd.year, TRUE, TRUE, 0);
	gtk_widget_show_all(box);
	gtk_container_add(GTK_CONTAINER(container), box);
	_builder_dialog_buttons(dialog, conf);
	ret = _builder_dialog_run(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			if(opt->date_fmt != NULL)
				fmt = opt->date_fmt;
			memset(&tm, 0, sizeof(tm));
			tm.tm_mday = gtk_spin_button_get_value_as_int(
					GTK_SPIN_BUTTON(dd.day));
			tm.tm_mon = gtk_combo_box_get_active(GTK_COMBO_BOX(
						dd.month));
			tm.tm_year = gtk_spin_button_get_value_as_int(
					GTK_SPIN_BUTTON(dd.year)) - 1900;
			len = strftime(buf, sizeof(buf) - 1, fmt, &tm);
			buf[len] = '\n';
			write(opt->output_fd, buf, len + 1);
			break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}

static void _datebox_on_year_value_changed(GtkWidget * widget)
{
	/* there is no year zero */
	if(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget)) == 0)
		/* FIXME this may make it look like year -1 can't be set */
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), 1.0);
}


/* builder_gauge */
static gboolean _gauge_on_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data);
static gboolean _gauge_on_can_read_eof(gpointer data);
static void _gauge_set_percentage(struct gauge_data * gd, unsigned int perc);

int builder_gauge(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	struct gauge_data gd = { NULL, NULL, NULL, 0, -1 };
	unsigned int perc = 0;
	GtkWidget * container;
	GIOChannel * channel;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, argc);
#endif
	if(argc > 1)
	{
		error_args(opt->name, argc - 1, argv + 1);
		return BSDDIALOG_ERROR;
	}
	else if(argc == 1)
		perc = strtoul(argv[0], NULL, 10);
	gd.dialog = _builder_dialog(conf, NULL, rows);
	container = gtk_dialog_get_content_area(GTK_DIALOG(gd.dialog));
	if(text != NULL)
	{
		gd.label = gtk_label_new(text);
		gtk_label_set_line_wrap(GTK_LABEL(gd.label), TRUE);
		gtk_label_set_line_wrap_mode(GTK_LABEL(gd.label),
				PANGO_WRAP_WORD_CHAR);
		gtk_label_set_single_line_mode(GTK_LABEL(gd.label), FALSE);
#if GTK_CHECK_VERSION(3, 10, 0)
		if(rows > 0)
			gtk_label_set_lines(GTK_LABEL(gd.label), rows);
#endif
		gtk_widget_show(gd.label);
		gtk_container_add(GTK_CONTAINER(container), gd.label);
	}
	gd.widget = gtk_progress_bar_new();
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(gd.widget), TRUE);
#endif
	_gauge_set_percentage(&gd, perc);
	gtk_widget_show(gd.widget);
	gtk_container_add(GTK_CONTAINER(container), gd.widget);
	channel = g_io_channel_unix_new(STDIN_FILENO);
	g_io_channel_set_encoding(channel, NULL, NULL);
	/* XXX ignore errors */
	g_io_channel_set_flags(channel, g_io_channel_get_flags(channel)
			| G_IO_FLAG_NONBLOCK, NULL);
	gd.id = g_io_add_watch(channel, G_IO_IN, _gauge_on_can_read, &gd);
	ret = _builder_dialog_run(gd.dialog);
	if(gd.id != 0)
		g_source_remove(gd.id);
	gtk_widget_destroy(gd.dialog);
	return ret;
}

static gboolean _gauge_on_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data)
{
	struct gauge_data * gd = data;
	const char sep[] = "XXX\n";
	const char end[] = "EOF\n";
	GIOStatus status;
	char buf[1024];
	gsize r;
	GError * error = NULL;
	char * p, * q;
	unsigned int perc;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(condition != G_IO_IN)
	{
		_builder_dialog_error(gd->dialog, NULL, "Unexpected condition");
		return _gauge_on_can_read_eof(gd);
	}
	if((status = g_io_channel_read_chars(channel, buf, sizeof(buf) - 1,
					&r, &error)) == G_IO_ERROR)
	{
		_builder_dialog_error(gd->dialog, NULL, error->message);
		g_error_free(error);
		return _gauge_on_can_read_eof(gd);
	}
	else if(status == G_IO_STATUS_AGAIN)
		return TRUE;
	else if(status == G_IO_STATUS_EOF)
		return _gauge_on_can_read_eof(gd);
	buf[r] = '\0';
	/* XXX the following parser assumes full lines are always obtained */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() buf=\"%s\"\n", __func__, buf);
#endif
	for(p = buf; p[0] != '\0'; p++)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() %d \"%s\"\n", __func__, gd->sep, p);
#endif
		if(strncmp(p, end, sizeof(end) - 1) == 0)
			return _gauge_on_can_read_eof(gd);
		if(strncmp(p, sep, sizeof(sep) - 1) == 0)
			/* found a separator */
			gd->sep = 0;
		else if(gd->sep == 0 && sscanf(p, "%u", &perc) == 1)
		{
			/* set the current percentage */
			_gauge_set_percentage(gd, perc);
			gd->sep = 1;
		}
		else if(gd->sep == 1 && (q = strchr(p, '\n')) != NULL)
		{
			/* set the current text */
			*q = '\0';
			gtk_label_set_text(GTK_LABEL(gd->label), p);
			gd->sep = -1;
			p = q;
			continue;
		}
		if((p = strchr(p, '\n')) == NULL)
			break;
	}
	return TRUE;
}

static gboolean _gauge_on_can_read_eof(gpointer data)
{
	struct gauge_data * gd = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	gtk_dialog_response(GTK_DIALOG(gd->dialog), GTK_RESPONSE_CLOSE);
	gd->id = 0;
	return FALSE;
}

static void _gauge_set_percentage(struct gauge_data * gd, unsigned int perc)
{
	gdouble fraction;
	char buf[8];

	perc = MIN(perc, 100);
	fraction = (gdouble)perc / 100.0;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(gd->widget), fraction);
	snprintf(buf, sizeof(buf), "%u %%", perc);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(gd->widget), buf);
}


/* builder_infobox */
static gboolean _infobox_on_key_press(GtkWidget * widget, GdkEventKey * event,
		gpointer data);
static gboolean _infobox_on_timeout(gpointer data);

int builder_infobox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	GtkWidget * widget;
#if GTK_CHECK_VERSION(3, 12, 0)
	const GtkDialogFlags flags = GTK_DIALOG_USE_HEADER_BAR;
#else
	const GtkDialogFlags flags = 0;
#endif
	GtkButtonsType buttons = GTK_BUTTONS_OK;
	struct infobox_data id = { NULL, 0 };

	if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	if(conf->sleep > 0)
	{
		buttons = GTK_BUTTONS_NONE;
		id.id = g_timeout_add(conf->sleep * 1000,
				_infobox_on_timeout, &id);
	}
	id.dialog = gtk_message_dialog_new(NULL, flags, GTK_MESSAGE_INFO,
			buttons, "%s", "Information");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(id.dialog),
			"%s", text);
	if(conf->key.enable_esc == false)
		gtk_window_set_deletable(GTK_WINDOW(id.dialog), FALSE);
	if(conf->key.f1_message != NULL)
		g_signal_connect(id.dialog, "key-press-event",
				G_CALLBACK(_infobox_on_key_press),
				(void *)conf);
	if(conf->title != NULL)
		gtk_window_set_title(GTK_WINDOW(id.dialog), conf->title);
	else
		gtk_window_set_title(GTK_WINDOW(id.dialog), "Information");
#if GTK_CHECK_VERSION(3, 12, 0)
	if(conf->bottomtitle != NULL
			&& (widget = gtk_dialog_get_header_bar(
					GTK_DIALOG(id.dialog))) != NULL)
		gtk_header_bar_set_subtitle(GTK_HEADER_BAR(widget),
				conf->bottomtitle);
#endif
	if(conf->x == BSDDIALOG_FULLSCREEN || conf->y == BSDDIALOG_FULLSCREEN)
		gtk_window_fullscreen(GTK_WINDOW(id.dialog));
	else if(conf->x > 0 && conf->y > 0)
		gtk_window_move(GTK_WINDOW(id.dialog), conf->x, conf->y);
	else
		gtk_window_set_position(GTK_WINDOW(id.dialog),
				GTK_WIN_POS_CENTER);
	gtk_dialog_run(GTK_DIALOG(id.dialog));
	if(id.id != 0)
		g_source_remove(id.id);
	return 0;
}

static gboolean _infobox_on_key_press(GtkWidget * widget, GdkEventKey * event,
		gpointer data)
{
	struct bsddialog_conf const * conf = data;

	if(event->keyval == GDK_KEY_F1)
		_builder_dialog_help(widget, conf);
	return FALSE;
}

static gboolean _infobox_on_timeout(gpointer data)
{
	struct infobox_data * id = data;

	gtk_dialog_response(GTK_DIALOG(id->dialog), GTK_RESPONSE_CLOSE);
	id->id = 0;
	return FALSE;
}


/* builder_inputbox */
int builder_inputbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * widget;
	GtkEntryBuffer * buffer;

	if(argc > 1)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, text, rows);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	buffer = gtk_entry_buffer_new(argc == 1 ? argv[0] : NULL, -1);
	widget = gtk_entry_new_with_buffer(buffer);
	if(cols > 0)
		gtk_entry_set_width_chars(GTK_ENTRY(widget), cols);
	gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	gtk_widget_show(widget);
	gtk_container_add(GTK_CONTAINER(container), widget);
	_builder_dialog_buttons(dialog, conf);
	ret = _builder_dialog_run(dialog);
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			dprintf(opt->output_fd, "%s\n",
					gtk_entry_buffer_get_text(buffer));
			break;
		default:
			ret = _builder_dialog_output(conf, opt, ret);
			break;
	}
	g_object_unref(buffer);
	return ret;
}


/* builder_menu */
static void _menu_on_row_activated(gpointer data);

int builder_menu(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * window;
	GtkWidget * widget;
	GtkListStore * store;
	GtkTreeIter iter;
	GtkTreeViewColumn * column;
	GtkTreeSelection * treesel;
	int i, j, n, res;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %d, %d (%d), \"%s\")\n", __func__, rows, cols,
			argc, (argc - 1) / 2,
			(argv[0] != NULL) ? argv[0] : "(null)");
#endif
	j = opt->item_bottomdesc ? 3 : 2;
	if(argc < 1 || (n = strtol(argv[0], NULL, 10)) > (argc - 1) / j)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	else if(n == 0)
		n = (argc - 1) / j;
	dialog = _builder_dialog(conf, text, rows);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	store = gtk_list_store_new(j, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING);
	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	widget = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(widget), GTK_TREE_MODEL(store));
	if(opt->item_bottomdesc)
		gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(widget), 2);
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_BROWSE);
	for(i = 0; i < n; i++)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				0, argv[i * j + 1], 1, argv[i * j + 2],
				(j == 3) ? 2 : -1,
				(j == 3) ? argv[i * j + 3] : NULL, -1);
		if(opt->item_default != NULL && strcmp(argv[i * j + 1],
					opt->item_default) == 0)
			gtk_tree_selection_select_iter(treesel, &iter);
	}
	if(conf->menu.no_name == false)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(), "text", 0, NULL);
		gtk_tree_view_column_set_expand(column, FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	if(conf->menu.no_desc == false)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(), "text", 1, NULL);
		gtk_tree_view_column_set_expand(column, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	g_signal_connect_swapped(widget, "row-activated",
			G_CALLBACK(_menu_on_row_activated), dialog);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 0);
	gtk_widget_show_all(window);
	_builder_dialog_buttons(dialog, conf);
	res = _builder_dialog_run(dialog);
	switch(res)
	{
		case BSDDIALOG_HELP:
			dprintf(opt->output_fd, "HELP ");
			/* fallback */
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			if(gtk_tree_selection_get_selected(treesel, NULL, &iter)
					== TRUE)
			{
				gtk_tree_model_get(GTK_TREE_MODEL(store), &iter,
						0, &p, -1);
				dprintf(opt->output_fd, "%s\n", p);
				free(p);
				gtk_widget_destroy(dialog);
				return res;
			}
			break;
	}
	gtk_widget_destroy(dialog);
	return _builder_dialog_output(conf, opt, res);
}

static void _menu_on_row_activated(gpointer data)
{
	GtkWidget * dialog = data;

	gtk_window_activate_default(GTK_WINDOW(dialog));
}


/* builder_msgbox */
int builder_msgbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	GtkWidget * dialog;
	int res;

	if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, text, rows);
	_builder_dialog_buttons(dialog, conf);
	res = _builder_dialog_run(dialog);
	gtk_widget_destroy(dialog);
	return _builder_dialog_output(conf, opt, res);
}


/* builder_passwordbox */
int builder_passwordbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * widget;
	GtkEntryBuffer * buffer;

	if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, text, rows);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	buffer = gtk_entry_buffer_new(NULL, -1);
	widget = gtk_entry_new_with_buffer(buffer);
	gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
	if(cols > 0)
		gtk_entry_set_width_chars(GTK_ENTRY(widget), cols);
	gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	gtk_widget_show(widget);
	gtk_container_add(GTK_CONTAINER(container), widget);
	_builder_dialog_buttons(dialog, conf);
	ret = _builder_dialog_run(dialog);
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			dprintf(opt->output_fd, "%s\n",
					gtk_entry_buffer_get_text(buffer));
			break;
		default:
			ret = _builder_dialog_output(conf, opt, ret);
			break;
	}
	g_object_unref(buffer);
	return ret;
}


/* builder_pause */
static gboolean _pause_on_timeout(gpointer data);

int builder_pause(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	struct pause_data pd;
	GtkWidget * container;

	if(argc == 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	if(argc > 1)
	{
		error_args(opt->name, argc - 1, argv + 1);
		return BSDDIALOG_ERROR;
	}
	pd.secs = strtoul(argv[0], NULL, 10);
	pd.dialog = _builder_dialog(conf, text, rows);
	container = gtk_dialog_get_content_area(GTK_DIALOG(pd.dialog));
	pd.widget = gtk_progress_bar_new();
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(pd.widget), TRUE);
#endif
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pd.widget), argv[0]);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pd.widget), 1.0);
	pd.step = (pd.secs > 0) ? 1.0 / (gdouble) pd.secs : 1.0;
	gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(pd.widget), pd.step);
	gtk_widget_show(pd.widget);
	gtk_container_add(GTK_CONTAINER(container), pd.widget);
	pd.id = g_timeout_add(1000, _pause_on_timeout, &pd);
	_builder_dialog_buttons(pd.dialog, conf);
	ret = _builder_dialog_run(pd.dialog);
	gtk_widget_destroy(pd.dialog);
	if(pd.id != 0)
		g_source_remove(pd.id);
	else
		ret = BSDDIALOG_TIMEOUT;
	return ret;
}

static gboolean _pause_on_timeout(gpointer data)
{
	struct pause_data * pd = data;
	gdouble fraction;
	char buf[16];

	fraction = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(pd->widget));
	if(fraction - pd->step < 0.0)
	{
#if GTK_CHECK_VERSION(3, 10, 0)
		gtk_window_close(GTK_WINDOW(pd->dialog));
#else
		gtk_dialog_response(GTK_DIALOG(pd->dialog), GTK_RESPONSE_CLOSE);
#endif
		pd->id = 0;
		return FALSE;
	}
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pd->widget),
			fraction - pd->step);
	snprintf(buf, sizeof(buf), "%u", --pd->secs);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pd->widget), buf);
	return TRUE;
}


/* builder_radiolist */
static void _radiolist_on_row_activated(GtkWidget * widget, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data);
static void _radiolist_on_row_toggled(GtkCellRenderer * renderer, char * path,
		gpointer data);

int builder_radiolist(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * window;
	GtkWidget * widget;
	GtkListStore * store;
	GtkTreeIter iter;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	GtkTreeSelection * treesel;
	int i, j, n;
	gboolean b, set;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %d, %d (%d), \"%s\")\n", __func__, rows, cols,
			argc, (argc - 1) / 3,
			(argv[0] != NULL) ? argv[0] : "(null)");
#endif
	j = opt->item_bottomdesc ? 4 : 3;
	if(argc < 1)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	if((n = strtol(argv[0], NULL, 10)) > (argc - 1) / j)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	else if(n == 0)
		n = (argc - 1) / j;
	dialog = _builder_dialog(conf, text, rows);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	store = gtk_list_store_new(j, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	widget = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(widget), GTK_TREE_MODEL(store));
	if(opt->item_bottomdesc)
		gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(widget), 3);
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_BROWSE);
	for(i = 0, set = FALSE; i < n; i++)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				0, set == FALSE && (set = strcasecmp(
						argv[i * j + 3], "on") == 0)
				? TRUE : FALSE,
				1, argv[i * j + 1], 2, argv[i * j + 2],
				(j == 4) ? 3 : -1,
				(j == 4) ? argv[i * j + 4] : NULL, -1);
		if(opt->item_default != NULL && strcmp(argv[i * j + 1],
					opt->item_default) == 0)
			gtk_tree_selection_select_iter(treesel, &iter);
	}
	renderer = gtk_cell_renderer_toggle_new();
	gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(renderer),
			TRUE);
	g_signal_connect(renderer, "toggled",
			G_CALLBACK(_radiolist_on_row_toggled), store);
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
			"active", 0, NULL);
	gtk_tree_view_column_set_expand(column, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	if(conf->menu.no_name == false)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(), "text", 1, NULL);
		gtk_tree_view_column_set_expand(column, FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	if(conf->menu.no_desc == false)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(), "text", 2, NULL);
		gtk_tree_view_column_set_expand(column, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	g_signal_connect(widget, "row-activated",
			G_CALLBACK(_radiolist_on_row_activated), NULL);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 0);
	gtk_widget_show_all(window);
	_builder_dialog_buttons(dialog, conf);
	ret = _builder_dialog_run(dialog);
	switch(ret)
	{
		case BSDDIALOG_HELP:
			if(gtk_tree_selection_get_selected(treesel, NULL, &iter)
					== FALSE)
				break;
			gtk_tree_model_get(GTK_TREE_MODEL(store), &iter,
					1, &p, -1);
			dprintf(opt->output_fd, "HELP %s\n", p);
			free(p);
			break;
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			for(b = gtk_tree_model_get_iter_first(
						GTK_TREE_MODEL(store), &iter);
					b != FALSE;
					b = gtk_tree_model_iter_next(
						GTK_TREE_MODEL(store), &iter))
			{
				gtk_tree_model_get(GTK_TREE_MODEL(store), &iter,
						0, &set, 1, &p, -1);
				if(set)
					dprintf(opt->output_fd, "%s\n", p);
				free(p);
			}
			break;
		default:
			_builder_dialog_output(conf, opt, ret);
			break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}

static void _radiolist_on_row_activated(GtkWidget * widget, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data)
{
	GtkTreeModel * model;
	GtkTreeIter iter;
	gboolean b;
	(void) column;
	(void) data;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	for(b = gtk_tree_model_get_iter_first(model, &iter); b;
			b = gtk_tree_model_iter_next(model, &iter))
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, FALSE, -1);
	if(gtk_tree_model_get_iter(model, &iter, path) == FALSE)
		return;
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, TRUE, -1);
}

static void _radiolist_on_row_toggled(GtkCellRenderer * renderer, char * path,
		gpointer data)
{
	GtkListStore * store = data;
	GtkTreePath * tp;
	GtkTreeIter iter;
	gboolean b;

	if((tp = gtk_tree_path_new_from_string(path)) == NULL)
		return;
	for(b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter); b;
			b = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter))
		gtk_list_store_set(store, &iter, 0, FALSE, -1);
	b = gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, tp);
	gtk_tree_path_free(tp);
	if(b == FALSE)
		return;
	gtk_list_store_set(store, &iter, 0, gtk_cell_renderer_toggle_get_active(
				GTK_CELL_RENDERER_TOGGLE(renderer))
			? FALSE : TRUE, -1);
}


/* builder_textbox */
int builder_textbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * window;
	GtkWidget * widget;
	GtkTextBuffer * buffer;
	GtkTextIter iter;
	FILE * fp;
	char buf[4096];
	size_t size;

	if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	/* FIXME use a GIOChannel instead */
	if((fp = fopen(text, "r")) == NULL)
		return BSDDIALOG_ERROR;
	dialog = _builder_dialog(conf, NULL, rows);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	widget = gtk_text_view_new();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(widget), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(widget), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widget), GTK_WRAP_WORD_CHAR);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
	gtk_text_buffer_get_start_iter(buffer, &iter);
	while((size = fread(buf, sizeof(char), sizeof(buf), fp)) > 0)
		gtk_text_buffer_insert(buffer, &iter, buf, size);
	fclose(fp);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 0);
	gtk_widget_show_all(window);
	gtk_dialog_add_button(GTK_DIALOG(dialog), "Exit", GTK_RESPONSE_OK);
#if GTK_CHECK_VERSION(3, 12, 0)
	if((widget = gtk_dialog_get_header_bar(GTK_DIALOG(dialog))) != NULL)
		gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(widget),
				FALSE);
#endif
	ret = _builder_dialog_run(dialog);
	gtk_widget_destroy(dialog);
	return ret;
}


/* builder_timebox */
static gboolean _timebox_on_idle(gpointer data);
static void _timebox_on_value_changed(GtkWidget * widget);

int builder_timebox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	struct timebox_data td = { NULL, NULL, NULL };
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * box;
	guint hour, minute, second;
	time_t t;
	struct tm tm;
	char const * fmt = "%H:%M:%S";
	char buf[1024];
	size_t len;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, argc);
#endif
	memset(&tm, 0, sizeof(tm));
	/* for more accurate time representation (eg leap seconds) */
	if((t = time(NULL)) == (time_t)-1 || localtime_r(&t, &tm) == NULL)
	{
		/* FIXME report error */
		return BSDDIALOG_ERROR;
	}
	if(argc == 3)
	{
		hour = strtoul(argv[0], NULL, 10);
		minute = strtoul(argv[1], NULL, 10);
		second = strtoul(argv[2], NULL, 10);
	}
	else if(argc > 0)
	{
		error_args(opt->name, argc - 1, argv + 1);
		return BSDDIALOG_ERROR;
	}
	else
	{
		hour = tm.tm_hour;
		minute = tm.tm_min;
		second = tm.tm_sec;
	}
	dialog = _builder_dialog(conf, text, rows);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	td.hour = gtk_spin_button_new_with_range(0.0, 23.0, 1.0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(td.hour), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(td.hour), (gdouble)hour);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(td.hour), TRUE);
	g_signal_connect(td.hour, "value-changed",
			G_CALLBACK(_timebox_on_value_changed), NULL);
	gtk_box_pack_start(GTK_BOX(box), td.hour, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box),
			gtk_label_new(":"), FALSE, TRUE, 4);
	td.minute = gtk_spin_button_new_with_range(0.0, 59.0, 1.0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(td.minute), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(td.minute), (gdouble)minute);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(td.minute), TRUE);
	g_signal_connect(td.minute, "value-changed",
			G_CALLBACK(_timebox_on_value_changed), NULL);
	gtk_box_pack_start(GTK_BOX(box), td.minute, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box),
			gtk_label_new(":"), FALSE, TRUE, 4);
	td.second = gtk_spin_button_new_with_range(0.0, 60.0, 1.0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(td.second), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(td.second), (gdouble)second);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(td.second), TRUE);
	g_signal_connect(td.second, "value-changed",
			G_CALLBACK(_timebox_on_value_changed), NULL);
	gtk_box_pack_start(GTK_BOX(box), td.second, TRUE, TRUE, 0);
	gtk_widget_show_all(box);
	gtk_container_add(GTK_CONTAINER(container), box);
	_builder_dialog_buttons(dialog, conf);
	td.id = g_idle_add(_timebox_on_idle, &td);
	ret = _builder_dialog_run(dialog);
	if(td.id != 0)
		g_source_remove(td.id);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			if(opt->time_fmt != NULL)
				fmt = opt->time_fmt;
			tm.tm_hour = gtk_spin_button_get_value_as_int(
					GTK_SPIN_BUTTON(td.hour));
			tm.tm_min = gtk_spin_button_get_value_as_int(
					GTK_SPIN_BUTTON(td.minute));
			tm.tm_sec = gtk_spin_button_get_value_as_int(
					GTK_SPIN_BUTTON(td.second));
			len = strftime(buf, sizeof(buf) - 1, fmt, &tm);
			buf[len] = '\n';
			write(opt->output_fd, buf, len + 1);
			break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}

static gboolean _timebox_on_idle(gpointer data)
{
	struct timebox_data * td = data;

	_timebox_on_value_changed(td->hour);
	_timebox_on_value_changed(td->minute);
	_timebox_on_value_changed(td->second);
	td->id = 0;
	return FALSE;
}

static void _timebox_on_value_changed(GtkWidget * widget)
{
	char buf[3];

	snprintf(buf, sizeof(buf), "%02u", gtk_spin_button_get_value_as_int(
				GTK_SPIN_BUTTON(widget)));
	gtk_entry_set_text(GTK_ENTRY(widget), buf);
}


/* builder_yesno */
int builder_yesno(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;

	if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, text, rows);
	if(conf->button.without_cancel != true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.cancel_label != NULL)
				? conf->button.cancel_label : "No",
				GTK_RESPONSE_NO);
	if(conf->button.with_extra == true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.extra_label != NULL)
				? conf->button.extra_label : "Extra",
				BSDDIALOG_EXTRA);
	if(conf->button.without_ok != true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.ok_label != NULL)
				? conf->button.ok_label : "Yes",
				GTK_RESPONSE_YES);
	if(conf->button.with_help == true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.help_label != NULL)
				? conf->button.help_label : "Help",
				GTK_RESPONSE_HELP);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
			conf->button.default_cancel
			? GTK_RESPONSE_NO : GTK_RESPONSE_YES);
	ret = _builder_dialog_run(dialog);
	gtk_widget_destroy(dialog);
	return ret;
}


/* builder_dialog */
static gboolean _dialog_on_key_press(GtkWidget * widget, GdkEventKey * event,
		gpointer data);

static GtkWidget * _builder_dialog(struct bsddialog_conf const * conf,
		char const * text, int rows)
{
	GtkWidget * dialog;
#if GTK_CHECK_VERSION(3, 12, 0)
	const GtkDialogFlags flags = GTK_DIALOG_USE_HEADER_BAR;
#else
	const GtkDialogFlags flags = 0;
#endif
	GtkWidget * container;
	GtkWidget * widget;

	dialog = gtk_dialog_new_with_buttons(conf->title, NULL, flags, NULL);
	if(conf->key.enable_esc == false)
		gtk_window_set_deletable(GTK_WINDOW(dialog), FALSE);
	if(conf->key.f1_message != NULL)
		g_signal_connect(dialog, "key-press-event",
				G_CALLBACK(_dialog_on_key_press), (void *)conf);
#if GTK_CHECK_VERSION(3, 12, 0)
	if(conf->bottomtitle != NULL
			&& (widget = gtk_dialog_get_header_bar(
					GTK_DIALOG(dialog))) != NULL)
		gtk_header_bar_set_subtitle(GTK_HEADER_BAR(widget),
				conf->bottomtitle);
#endif
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	if(text != NULL)
	{
		widget = gtk_label_new(text);
		gtk_label_set_line_wrap(GTK_LABEL(widget), TRUE);
		gtk_label_set_line_wrap_mode(GTK_LABEL(widget),
				PANGO_WRAP_WORD_CHAR);
		gtk_label_set_single_line_mode(GTK_LABEL(widget), FALSE);
#if GTK_CHECK_VERSION(3, 10, 0)
		if(rows > 0)
			gtk_label_set_lines(GTK_LABEL(widget), rows);
#endif
		gtk_widget_show(widget);
		gtk_container_add(GTK_CONTAINER(container), widget);
	}
	if(conf->x == BSDDIALOG_FULLSCREEN || conf->y == BSDDIALOG_FULLSCREEN)
		gtk_window_fullscreen(GTK_WINDOW(dialog));
	else if(conf->x > 0 && conf->y > 0)
		gtk_window_move(GTK_WINDOW(dialog), conf->x, conf->y);
	else
		gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	return dialog;
}

static gboolean _dialog_on_key_press(GtkWidget * widget, GdkEventKey * event,
		gpointer data)
{
	struct bsddialog_conf const * conf = data;

	if(event->keyval == GDK_KEY_F1)
		_builder_dialog_help(widget, conf);
	return FALSE;
}


/* builder_dialog_buttons */
static void _builder_dialog_buttons(GtkWidget * dialog,
		struct bsddialog_conf const * conf)
{
	if(conf->button.without_cancel != true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.cancel_label != NULL)
				? conf->button.cancel_label : "Cancel",
				GTK_RESPONSE_CANCEL);
	if(conf->button.with_extra == true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.extra_label != NULL)
				? conf->button.extra_label : "Extra",
				BSDDIALOG_EXTRA);
	if(conf->button.without_ok != true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.ok_label != NULL)
				? conf->button.ok_label : "OK", GTK_RESPONSE_OK);
	if(conf->button.with_help == true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.help_label != NULL)
				? conf->button.help_label : "Help",
				GTK_RESPONSE_HELP);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
			conf->button.default_cancel
			? GTK_RESPONSE_CANCEL : GTK_RESPONSE_OK);
}


/* builder_dialog_error */
static int _builder_dialog_error(GtkWidget * parent,
		struct bsddialog_conf const * conf, char const * error)
{
	GtkWidget * dialog;
#if GTK_CHECK_VERSION(3, 12, 0)
	const GtkDialogFlags flags = GTK_DIALOG_USE_HEADER_BAR;
#else
	const GtkDialogFlags flags = 0;
#endif

	dialog = gtk_message_dialog_new(parent ? GTK_WINDOW(parent) : NULL,
			flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
			"%s", "Error");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s", error);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	if(parent == NULL && conf != NULL)
	{
		if(conf->x == BSDDIALOG_FULLSCREEN
				|| conf->y == BSDDIALOG_FULLSCREEN)
			gtk_window_fullscreen(GTK_WINDOW(dialog));
		else if(conf->x > 0 && conf->y > 0)
			gtk_window_move(GTK_WINDOW(dialog), conf->x, conf->y);
		else
			gtk_window_set_position(GTK_WINDOW(dialog),
					GTK_WIN_POS_CENTER);
	}
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return BSDDIALOG_ERROR;
}


/* builder_dialog_help */
static int _builder_dialog_help(GtkWidget * parent,
		struct bsddialog_conf const * conf)
{
	int ret;
	GtkWidget * dialog;
	const GtkDialogFlags flags = 0;
	GtkButtonsType buttons = GTK_BUTTONS_CLOSE;

	dialog = gtk_message_dialog_new((parent != NULL)
			? GTK_WINDOW(parent) : NULL, flags,
			GTK_MESSAGE_QUESTION, buttons, "%s", "Help");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s", conf->key.f1_message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Help");
	ret = _builder_dialog_run(dialog);
	gtk_widget_destroy(dialog);
	return ret;
}


/* builder_dialog_output */
static int _builder_dialog_output(struct bsddialog_conf const * conf,
		struct options const * opt, int res)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, res);
#endif
	switch(res)
	{
		case BSDDIALOG_CANCEL:
			dprintf(opt->output_fd, "%s\n",
					conf->button.cancel_label
					? conf->button.cancel_label : "Cancel");
			break;
		case BSDDIALOG_ESC:
			dprintf(opt->output_fd, "%s\n", "[ESC]");
			break;
		case BSDDIALOG_EXTRA:
			dprintf(opt->output_fd, "%s\n", conf->button.extra_label
					? conf->button.extra_label : "Extra");
			break;
		case BSDDIALOG_OK:
			dprintf(opt->output_fd, "%s\n", conf->button.ok_label
					? conf->button.ok_label : "OK");
			break;
	}
	return res;
}


/* builder_dialog_run */
static int _builder_dialog_run(GtkWidget * dialog)
{
	int res;

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);
	switch(res)
	{
		case GTK_RESPONSE_CANCEL:
		case GTK_RESPONSE_NO:
			return BSDDIALOG_CANCEL;
		case GTK_RESPONSE_CLOSE:
		case GTK_RESPONSE_DELETE_EVENT:
			return BSDDIALOG_ESC;
		case GTK_RESPONSE_HELP:
			return BSDDIALOG_HELP;
		case GTK_RESPONSE_OK:
		case GTK_RESPONSE_YES:
			return BSDDIALOG_OK;
		case BSDDIALOG_EXTRA:
			return res;
	}
	return BSDDIALOG_ERROR;
}
