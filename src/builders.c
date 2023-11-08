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
#include "callbacks.h"
#include "builders.h"

#ifndef MIN
# define MIN(a, b) ((a) <= (b) ? (a) : (b))
#endif


/* builders */
/* types */
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


/* prototypes */
static GtkWidget * _builder_dialog(struct bsddialog_conf const * conf,
		char const * text, int rows);
static void _builder_dialog_buttons(GtkWidget * dialog,
		struct bsddialog_conf const * conf);
static int _builder_dialog_error(GtkWidget * parent, char const * error);
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
	struct tm t;
	char buf[1024];
	size_t len;

	if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, text, rows);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	widget = gtk_calendar_new();
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
			{
				memset(&t, 0, sizeof(t));
				t.tm_year = year - 1900;
				t.tm_mon = month;
				t.tm_mday = day;
				len = strftime(buf, sizeof(buf) - 1,
						opt->date_fmt, &t);
				buf[len] = '\n';
				write(opt->output_fd, buf, len + 1);
			}
			else
				dprintf(opt->output_fd, "%u/%u/%u\n", day, month + 1, year);
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
		gtk_list_store_insert(store, &iter, -1);
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
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 4);
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
	gtk_list_store_set(store, &iter, 0,
			gtk_cell_renderer_toggle_get_active(
				GTK_CELL_RENDERER_TOGGLE(renderer))
			? FALSE : TRUE, -1);
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
		if(rows > 0)
			gtk_label_set_lines(GTK_LABEL(gd.label), rows);
		gtk_widget_show(gd.label);
		gtk_container_add(GTK_CONTAINER(container), gd.label);
	}
	gd.widget = gtk_progress_bar_new();
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(gd.widget), TRUE);
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
		_builder_dialog_error(gd->dialog, "Unexpected condition");
		return _gauge_on_can_read_eof(gd);
	}
	if((status = g_io_channel_read_chars(channel, buf, sizeof(buf) - 1,
					&r, &error)) == G_IO_ERROR)
	{
		_builder_dialog_error(gd->dialog, error->message);
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
static gboolean _infobox_on_timeout(gpointer data);

int builder_infobox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	GtkWidget * widget;
	const GtkDialogFlags flags = GTK_DIALOG_USE_HEADER_BAR;
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
			buttons, "%s", text);
	if(conf->key.enable_esc == false)
		gtk_window_set_deletable(GTK_WINDOW(id.dialog), FALSE);
	gtk_window_set_position(GTK_WINDOW(id.dialog), GTK_WIN_POS_CENTER);
	if(conf->title != NULL)
		gtk_window_set_title(GTK_WINDOW(id.dialog), conf->title);
	if(conf->bottomtitle != NULL
			&& (widget = gtk_dialog_get_header_bar(
					GTK_DIALOG(id.dialog))) != NULL)
		gtk_header_bar_set_subtitle(GTK_HEADER_BAR(widget),
				conf->bottomtitle);
	gtk_dialog_run(GTK_DIALOG(id.dialog));
	if(id.id != 0)
		g_source_remove(id.id);
	return 0;
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
		gtk_list_store_insert(store, &iter, -1);
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
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 4);
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
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(pd.widget), TRUE);
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
		gtk_window_close(GTK_WINDOW(pd->dialog));
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
		gtk_list_store_insert(store, &iter, -1);
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
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 4);
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
	gtk_list_store_set(store, &iter, 0,
			gtk_cell_renderer_toggle_get_active(
				GTK_CELL_RENDERER_TOGGLE(renderer))
			? FALSE : TRUE, -1);
}


/* builder_yesno */
int builder_yesno(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	GtkWidget * dialog;
	int ret;

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
static GtkWidget * _builder_dialog(struct bsddialog_conf const * conf,
		char const * text, int rows)
{
	GtkWidget * dialog;
	const GtkDialogFlags flags = GTK_DIALOG_USE_HEADER_BAR;
	GtkWidget * container;
	GtkWidget * widget;

	dialog = gtk_dialog_new_with_buttons(conf->title, NULL, flags, NULL);
	if(conf->key.enable_esc == false)
		gtk_window_set_deletable(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	if(conf->bottomtitle != NULL
			&& (widget = gtk_dialog_get_header_bar(
					GTK_DIALOG(dialog))) != NULL)
		gtk_header_bar_set_subtitle(GTK_HEADER_BAR(widget),
				conf->bottomtitle);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	if(text != NULL)
	{
		widget = gtk_label_new(text);
		gtk_label_set_line_wrap(GTK_LABEL(widget), TRUE);
		gtk_label_set_line_wrap_mode(GTK_LABEL(widget),
				PANGO_WRAP_WORD_CHAR);
		gtk_label_set_single_line_mode(GTK_LABEL(widget), FALSE);
		if(rows > 0)
			gtk_label_set_lines(GTK_LABEL(widget), rows);
		gtk_widget_show(widget);
		gtk_container_add(GTK_CONTAINER(container), widget);
	}
	return dialog;
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
static int _builder_dialog_error(GtkWidget * parent, char const * error)
{
	GtkWidget * dialog;
	const GtkDialogFlags flags = GTK_DIALOG_USE_HEADER_BAR;

	dialog = gtk_message_dialog_new(parent ? GTK_WINDOW(parent) : NULL,
			flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
			"%s", "Error");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s", error);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return BSDDIALOG_ERROR;
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
