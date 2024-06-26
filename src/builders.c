/* gbsddialog */
/* builders.c */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023-2024 The FreeBSD Foundation
 *
 * The printing code is:
 * Copyright (c) 2015-2024 Pierre Pronchery <khorben@defora.org>
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



#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <time.h>
#ifdef WITH_XDIALOG
# include <math.h>
#endif
#include <errno.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "callbacks.h"
#include "common.h"
#include "builders.h"

#ifndef MIN
# define MIN(a, b) ((a) <= (b) ? (a) : (b))
#endif


/* builders */
/* types */
struct confopt_data
{
	struct bsddialog_conf const * conf;
	struct options const * opt;
};

struct datebox_data
{
	GtkWidget * day;
	GtkWidget * month;
	GtkWidget * year;
};

struct gauge_data
{
	struct options const * opt;
	GtkWidget * dialog;
	GtkWidget * label;
	GtkWidget * widget;	/* progress bar */
	guint id;
	int state;		/* -1 uninitialized, 0 initialized,
				    1 percentage read, 2 text,
				    3 text (continued) */
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

struct radiolist_data
{
	int depth;
	GtkTreeIter * parent;
	GtkTreeIter * pparent;
	char const * name;
};

struct textbox_data
{
	struct options const * opt;

	/* preferences */
	gboolean editable;
#ifdef WITH_XDIALOG
	gboolean scroll;
#endif

	char const * filename;
	int fd;
	GtkWidget * dialog;
	GtkWidget * view;
	GtkTextBuffer * buffer;
	GtkTextIter iter;
	guint id;
	GIOChannel * channel;

#ifdef WITH_XDIALOG
	/* printing */
	GtkWidget * button;
	PangoFontDescription * font;
	double font_size;
	double line_space;
	guint line_count;
#endif
};

struct timebox_data
{
	GtkWidget * hour;
	GtkWidget * minute;
	GtkWidget * second;
};


/* constants */
enum CHECKLIST_TREE_STORE
{
	CTS_PREFIX = 0,
	CTS_SET,
	CTS_DEPTH,
	CTS_NAME,
	CTS_DESCRIPTION,
	CTS_TOOLTIP
};
# define CTS_LAST CTS_TOOLTIP
# define CTS_COUNT (CTS_LAST + 1)

enum MENU_TREE_STORE
{
	MTS_PREFIX = 0,
	MTS_DEPTH,
	MTS_NAME,
	MTS_DESCRIPTION,
	MTS_TOOLTIP
};
# define MTS_LAST MTS_TOOLTIP
# define MTS_COUNT (MTS_LAST + 1)

enum RADIOLIST_TREE_STORE
{
	RTS_PREFIX = 0,
	RTS_SET,
	RTS_DEPTH,
	RTS_NAME,
	RTS_DESCRIPTION,
	RTS_TOOLTIP
};
# define RTS_LAST RTS_TOOLTIP
# define RTS_COUNT (RTS_LAST + 1)


/* prototypes */
static GtkWidget * _builder_dialog(struct bsddialog_conf const * conf,
		struct options const * opt, char const * text,
		int rows, int cols);
static void _builder_dialog_buttons(GtkWidget * dialog,
		struct bsddialog_conf const * conf,
		struct options const * opt);
static int _builder_dialog_error(GtkWidget * parent,
		struct bsddialog_conf const * conf, struct options const * opt,
		char const * error);
static int _builder_dialog_help(GtkWidget * parent,
		struct bsddialog_conf const * conf,
		struct options const * opt);
static int _builder_dialog_menu_output(struct options const * opt,
		GtkTreeSelection * treesel, GtkTreeModel * model,
		unsigned int id, char const * prefix);
static int _builder_dialog_run(struct bsddialog_conf const * conf,
		GtkWidget * dialog);


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
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
	widget = gtk_calendar_new();
	if(argc == 3 && day <= 31 && month >= 1 && month <= 12 && year != 0)
	{
		gtk_calendar_select_day(GTK_CALENDAR(widget), day);
		gtk_calendar_select_month(GTK_CALENDAR(widget), month - 1,
				year);
	}
	if(conf->button.always_active == true)
		g_signal_connect_swapped(widget, "day-selected-double-click",
				G_CALLBACK(_calendar_on_day_activated), dialog);
	gtk_box_pack_start(GTK_BOX(container), widget, TRUE, TRUE,
			BORDER_WIDTH);
	gtk_widget_show(widget);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
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
	return ret;
}

static void _calendar_on_day_activated(gpointer data)
{
	GtkWidget * dialog = data;

	gtk_window_activate_default(GTK_WINDOW(dialog));
}


/* builder_checklist */
static GtkTreeIter * _checklist_get_parent(GtkTreeModel * model,
		GtkTreeIter * iter, int depth);
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
	GtkTreeStore * store;
	GtkTreeIter iter, parent, * pparent = NULL;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	GtkTreeSelection * treesel;
	int i, j, k, n, depth = 0;
	gboolean b, set, toquote;
	char quotech;
	char const * prefix = NULL, * name, * desc, * tooltip;
	char * p, * sep = "";

	j = opt->item_bottomdesc ? 4 : 3;
	if(opt->item_prefix)
		j++;
	if(opt->item_depth)
		j++;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %d, %d (%d), \"%s\")\n", __func__,
			rows, cols, argc, (argc - 1) / j,
			(argv[0] != NULL) ? argv[0] : "(null)");
#endif
	if(argc < 1 || (n = strtol(argv[0], NULL, 10)) < 0
			|| ((argc - 1) % j) != 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	else if(n == 0)
		n = (argc - 1) / j;
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
	store = gtk_tree_store_new(CTS_COUNT, G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	if(conf->shadow == false)
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(window),
				GTK_SHADOW_NONE);
	widget = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(widget), GTK_TREE_MODEL(store));
	if(opt->item_bottomdesc)
		gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(widget),
				CTS_TOOLTIP);
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_BROWSE);
	for(i = 0; (i + 1) * j < argc; i++)
	{
		k = i * j + 1;
		if(opt->item_prefix)
			prefix = argv[k++];
		if(opt->item_depth)
		{
			depth = strtol(argv[k++], NULL, 10);
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() depth=%d\n", __func__,
					depth);
#endif
			pparent = _checklist_get_parent(GTK_TREE_MODEL(store),
					&parent, depth);
		}
		name = argv[k++];
		desc = argv[k++];
		set = (strcasecmp(argv[k++], "on") == 0) ? TRUE : FALSE;
		tooltip = (k < j) ? argv[k++] : NULL;
		gtk_tree_store_append(store, &iter, pparent);
		gtk_tree_store_set(store, &iter, CTS_PREFIX, prefix,
				CTS_SET, set, CTS_DEPTH, depth,
				CTS_NAME, name, CTS_DESCRIPTION, desc,
				(tooltip != NULL) ? CTS_TOOLTIP : -1,
				(tooltip != NULL) ? tooltip : NULL, -1);
		if(opt->item_default != NULL
				&& strcmp(name, opt->item_default) == 0)
			gtk_tree_selection_select_iter(treesel, &iter);
	}
	if(opt->item_prefix == true)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(),
				"text", CTS_PREFIX, NULL);
		gtk_tree_view_column_set_expand(column, FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	renderer = gtk_cell_renderer_toggle_new();
	g_signal_connect(renderer, "toggled",
			G_CALLBACK(_checklist_on_row_toggled), store);
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
			"active", CTS_SET, NULL);
	gtk_tree_view_column_set_expand(column, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	if(conf->menu.no_name == false)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(), "text", CTS_NAME,
				NULL);
		gtk_tree_view_column_set_expand(column, FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	if(conf->menu.no_desc == false)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(), "text",
				CTS_DESCRIPTION, NULL);
		gtk_tree_view_column_set_expand(column, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	if(opt->item_depth)
		gtk_tree_view_expand_all(GTK_TREE_VIEW(widget));
	g_signal_connect(widget, "row-activated",
			G_CALLBACK(_checklist_on_row_activated), NULL);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 0);
	gtk_widget_show_all(window);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	quotech = opt->item_singlequote ? '\'' : '"';
	switch(ret)
	{
		case BSDDIALOG_HELP:
			_builder_dialog_menu_output(opt, treesel,
					GTK_TREE_MODEL(store), CTS_NAME,
					"HELP ");
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
						CTS_SET, &set, CTS_NAME, &p,
						-1);
				if(set)
				{
					if(opt->item_output_sepnl == FALSE)
						toquote = TRUE;
					else if(string_needs_quoting(p))
						toquote = opt->item_always_quote;
					else
						toquote = FALSE;
					if(toquote)
						dprintf(opt->output_fd,
								"%s%c%s%c", sep,
								quotech, p,
								quotech);
					else
						dprintf(opt->output_fd, "%s%s",
								sep, p);
				}
				free(p);
				sep = (opt->item_output_sep != NULL)
					? opt->item_output_sep
					: (opt->item_output_sepnl ? "\n" : " ");
			}
			dprintf(opt->output_fd, "\n");
			break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}

static GtkTreeIter * _checklist_get_parent(GtkTreeModel * model,
		GtkTreeIter * parent, int depth)
{
	GtkTreeIter * ret = NULL;
	GtkTreeIter iter;
	gboolean b;
	int d;

	if(depth <= 0)
		return NULL;
	for(b = gtk_tree_model_get_iter_first(model, &iter); b == TRUE;
			b = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, CTS_DEPTH, &d, -1);
		if(d < depth)
		{
			*parent = iter;
			ret = parent;
		}
	}
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
	gtk_tree_model_get(model, &iter, CTS_SET, &set, -1);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			CTS_SET, set ? FALSE : TRUE, -1);
}

static void _checklist_on_row_toggled(GtkCellRenderer * renderer, char * path,
		gpointer data)
{
	GtkTreeStore * store = data;
	GtkTreePath * tp;
	GtkTreeIter iter;
	gboolean b;

	if((tp = gtk_tree_path_new_from_string(path)) == NULL)
		return;
	b = gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, tp);
	gtk_tree_path_free(tp);
	if(b == FALSE)
		return;
	gtk_tree_store_set(store, &iter, CTS_SET,
			gtk_cell_renderer_toggle_get_active(
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
		return _builder_dialog_error(NULL, conf, opt, strerror(errno));
	else
	{
		day = tm.tm_mday;
		month = tm.tm_mon + 1;
		year = tm.tm_year + 1900;
	}
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
#if GTK_CHECK_VERSION(3, 0, 0)
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, BORDER_WIDTH);
#else
	box = gtk_hbox_new(FALSE, BORDER_WIDTH);
#endif
	gtk_box_pack_start(GTK_BOX(box),
			gtk_label_new("Day: "), FALSE, TRUE, 0);
	dd.day = gtk_spin_button_new_with_range(1.0, 31.0, 1.0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(dd.day), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dd.day), (gdouble)day);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(dd.day), TRUE);
	gtk_box_pack_start(GTK_BOX(box), dd.day, TRUE, TRUE, 0);
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
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(dd.year), TRUE);
	g_signal_connect(dd.year, "value-changed",
			G_CALLBACK(_datebox_on_year_value_changed), NULL);
	gtk_box_pack_start(GTK_BOX(box), dd.year, TRUE, TRUE, 0);
	gtk_widget_show_all(box);
	gtk_container_add(GTK_CONTAINER(container), box);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
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


/* builder_form */
static void _form_foreach_buffer(gpointer e, gpointer data);

int builder_form(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * box;
	GtkWidget * widget;
	GtkEntryBuffer * buffer;
	GtkSizeGroup * group;
	int i, k, n, fieldlen, maxletters;
	GSList * l = NULL;
	const int j = 8;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, argc);
#endif
	if(argc < 1 || (n = strtol(argv[0], NULL, 10)) < 0
			|| ((argc - 1) % j) != 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	else if(n == 0)
		n = (argc - 1) / j;
	group = gtk_size_group_new(GTK_SIZE_GROUP_BOTH);
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
	gtk_box_set_spacing(GTK_BOX(container), BORDER_WIDTH);
	for(i = 0; (i + 1) * j < argc; i++)
	{
		k = i * j + 1;
#if GTK_CHECK_VERSION(3, 0, 0)
		box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, BORDER_WIDTH);
#else
		box = gtk_hbox_new(FALSE, BORDER_WIDTH);
#endif
		widget = gtk_label_new(argv[k]);
#if GTK_CHECK_VERSION(3, 14, 0)
		gtk_widget_set_halign(widget, opt->halign);
#else
		gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
#ifdef WITH_XDIALOG
		gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
#endif
		gtk_size_group_add_widget(group, widget);
		gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
		buffer = gtk_entry_buffer_new(argv[k + 3], -1);
		l = g_slist_append(l, buffer);
		widget = gtk_entry_new_with_buffer(buffer);
		if(conf->button.always_active == true)
			gtk_entry_set_activates_default(GTK_ENTRY(widget),
					TRUE);
		if(conf->form.securech != '\0')
			gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
		fieldlen = strtol(argv[k + 6], NULL, 10);
		maxletters = strtol(argv[k + 7], NULL, 10);
		if(fieldlen != 0)
			gtk_entry_set_width_chars(GTK_ENTRY(widget),
					abs(fieldlen));
		if(fieldlen < 0)
			gtk_editable_set_editable(GTK_EDITABLE(widget), FALSE);
		if(maxletters == 0)
			maxletters = abs(fieldlen);
		if(maxletters > 0)
			gtk_entry_set_max_length(GTK_ENTRY(widget), maxletters);
		gtk_box_pack_start(GTK_BOX(box), widget,
				(maxletters != 0) ? FALSE : TRUE, TRUE, 0);
		gtk_container_add(GTK_CONTAINER(container), box);
	}
	gtk_widget_show_all(container);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			g_slist_foreach(l, _form_foreach_buffer, (void *)opt);
			break;
	}
	g_slist_foreach(l, (GFunc)g_object_unref, NULL);
	g_slist_free(l);
	return ret;
}

static void _form_foreach_buffer(gpointer e, gpointer data)
{
	GtkEntryBuffer * buffer = e;
	struct options const * opt = data;

	dprintf(opt->output_fd, "%s\n", gtk_entry_buffer_get_text(buffer));
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
	struct gauge_data gd = { NULL, NULL, NULL, NULL, 0, -1 };
	unsigned int perc = 0;
	GtkWidget * container;
	GtkWidget * box;
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
	gd.opt = opt;
	gd.dialog = _builder_dialog(conf, opt, NULL, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(gd.dialog));
#else
	container = gd.dialog->vbox;
#endif
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
#if GTK_CHECK_VERSION(3, 14, 0)
		gtk_widget_set_halign(gd.label, opt->halign);
#else
		gtk_misc_set_alignment(GTK_MISC(gd.label), opt->halign, 0.5);
#endif
#ifdef WITH_XDIALOG
		gtk_label_set_justify(GTK_LABEL(gd.label), opt->justify);
#endif
		gtk_widget_show(gd.label);
		gtk_box_pack_start(GTK_BOX(container), gd.label, FALSE, TRUE,
				BORDER_WIDTH);
	}
	gd.widget = gtk_progress_bar_new();
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(gd.widget), TRUE);
#endif
	_gauge_set_percentage(&gd, perc);
#if GTK_CHECK_VERSION(3, 0, 0)
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, BORDER_WIDTH);
#else
	box = gtk_vbox_new(FALSE, BORDER_WIDTH);
#endif
	gtk_box_pack_start(GTK_BOX(box), gd.widget, FALSE, TRUE, 0);
	gtk_widget_show_all(box);
	gtk_container_add(GTK_CONTAINER(container), box);
	channel = g_io_channel_unix_new(STDIN_FILENO);
	g_io_channel_set_encoding(channel, NULL, NULL);
	/* XXX ignore errors */
	g_io_channel_set_flags(channel, g_io_channel_get_flags(channel)
			| G_IO_FLAG_NONBLOCK, NULL);
	gd.id = g_io_add_watch(channel, G_IO_IN, _gauge_on_can_read, &gd);
	ret = _builder_dialog_run(conf, gd.dialog);
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
	char buf[BUFSIZ + 1];
	gsize r;
	GError * error = NULL;
	char * p, * q, * s;
	char const * l;
	unsigned int perc;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(condition != G_IO_IN)
	{
		_builder_dialog_error(gd->dialog, NULL, NULL,
				"Unexpected condition");
		return _gauge_on_can_read_eof(gd);
	}
	if((status = g_io_channel_read_chars(channel, buf, sizeof(buf) - 1,
					&r, &error)) == G_IO_STATUS_ERROR)
	{
		_builder_dialog_error(gd->dialog, NULL, NULL, error->message);
		g_error_free(error);
		return _gauge_on_can_read_eof(gd);
	}
	else if(status == G_IO_STATUS_AGAIN)
		return TRUE;
	else if(status == G_IO_STATUS_EOF)
		return _gauge_on_can_read_eof(gd);
	buf[r] = '\0';
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() buf=\"%s\"\n", __func__, buf);
#endif
	for(p = buf; p[0] != '\0'; p++)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() %d \"%s\"\n",
				__func__, gd->state, p);
#endif
		if(gd->state != 3 && strncmp(p, end, sizeof(end) - 1) == 0)
			return _gauge_on_can_read_eof(gd);
		if(gd->state != 3 && strncmp(p, sep, sizeof(sep) - 1) == 0)
			/* found a separator */
			gd->state = 0;
		else if(gd->state == 0 && sscanf(p, "%u", &perc) == 1)
		{
			/* set the current percentage */
			_gauge_set_percentage(gd, perc);
			gtk_label_set_text(GTK_LABEL(gd->label), NULL);
			gd->state = 1;
		}
		else if(gd->state == 0 || gd->state == 1)
		{
			/* set the current text */
			if((q = strchr(p, '\n')) != NULL)
				*q = '\0';
			gtk_label_set_text(GTK_LABEL(gd->label), p);
			if(q == NULL)
			{
				gd->state = 3;
				break;
			}
			gd->state = 2;
			p = q;
			continue;
		}
		else if(gd->state == 2 || gd->state == 3)
		{
			/* append to the current text */
			if((q = strchr(p, '\n')) != NULL)
				*q = '\0';
			l = gtk_label_get_text(GTK_LABEL(gd->label));
			r = strlen(l) + strlen(p) + 2;
			if((s = malloc(r)) != NULL)
			{
				snprintf(s, r, "%s%s%s", l,
						(gd->state == 2) ? "\n" : "",
						p);
				gtk_label_set_text(GTK_LABEL(gd->label), s);
				free(s);
			}
			if(q == NULL)
			{
				gd->state = 3;
				break;
			}
			gd->state = 2;
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
	if(!gd->opt->ignore_eof)
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
	GtkWidget * container;
#if GTK_CHECK_VERSION(3, 12, 0)
	GtkWidget * widget;
	const GtkDialogFlags flags = GTK_DIALOG_USE_HEADER_BAR;
#else
	const GtkDialogFlags flags = 0;
#endif
#if defined(WITH_XDIALOG)
	GtkWidget * image;
	char const * p;
	int t;
#endif
	GtkButtonsType buttons = GTK_BUTTONS_NONE;
	gdouble ex;
	struct confopt_data confopt = { conf, opt };
	struct infobox_data id = { NULL, 0 };
	int timeout = (conf->sleep > INT_MAX) ? INT_MAX : (int)conf->sleep;

#ifdef WITH_XDIALOG
	if(opt->high_compat)
	{
		t = (p = getenv("XDIALOG_INFOBOX_TIMEOUT")) != NULL
			? strtol(p, NULL, 10) : 0;
		if(t == 0)
			return builder_msgbox(conf, text, rows, cols,
					argc, argv, opt);
		timeout = t;
	}
	if(argc == 1)
	{
		timeout = strtol(argv[0], NULL, 10);
		argc--;
	}
	else if((p = getenv("XDIALOG_INFOBOX_TIMEOUT")) != NULL
			&& (t = strtol(p, NULL, 10)) > 0)
		timeout = t;
#endif
	if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	if(timeout >= 0)
		id.id = g_timeout_add((timeout > 0) ? timeout : 1000,
				_infobox_on_timeout, &id);
	id.dialog = gtk_message_dialog_new(NULL, flags, GTK_MESSAGE_INFO,
			buttons,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", "Information");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(id.dialog),
#endif
			"%s", text);
#ifdef WITH_XDIALOG
	if(opt->icon != NULL)
	{
		image = gtk_image_new_from_file(opt->icon);
		gtk_message_dialog_set_image(GTK_MESSAGE_DIALOG(id.dialog),
				image);
		gtk_widget_show(image);
	}
#endif
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(id.dialog));
#else
	container = id.dialog->vbox;
#endif
	if(rows != BSDDIALOG_AUTOSIZE && cols != BSDDIALOG_AUTOSIZE)
	{
		/* XXX gdk_screen_get_default() may fail */
		ex = get_font_size(gdk_screen_get_default());
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() ex=%f cols=%d rows=%d\n", __func__,
				ex, cols, rows);
#endif
#ifdef WITH_XDIALOG
		if(opt->pixelsize)
			gtk_widget_set_size_request(container, cols, rows);
		else
#endif
			gtk_widget_set_size_request(container, (int)(cols * ex),
					(int)(rows * ex * 2.0));
	}
#ifdef WITH_XDIALOG
	if(opt->without_buttons)
		gtk_window_set_decorated(GTK_WINDOW(id.dialog), FALSE);
	if(opt->wmclass != NULL)
		gtk_window_set_wmclass(GTK_WINDOW(id.dialog), opt->wmclass,
				opt->wmclass);
#endif
	if(conf->key.enable_esc == false)
		gtk_window_set_deletable(GTK_WINDOW(id.dialog), FALSE);
	if(conf->key.f1_file != NULL || conf->key.f1_message != NULL)
		g_signal_connect(id.dialog, "key-press-event",
				G_CALLBACK(_infobox_on_key_press), &confopt);
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
	else if(conf->x != BSDDIALOG_AUTOSIZE && conf->y != BSDDIALOG_AUTOSIZE)
		gtk_window_move(GTK_WINDOW(id.dialog), conf->x, conf->y);
	else
		gtk_window_set_position(GTK_WINDOW(id.dialog), opt->position);
	_builder_dialog_run(conf, id.dialog);
	if(id.id != 0)
		g_source_remove(id.id);
	gtk_widget_destroy(id.dialog);
	return 0;
}

static gboolean _infobox_on_key_press(GtkWidget * widget, GdkEventKey * event,
		gpointer data)
{
	struct confopt_data * confopt = data;

	if(event->keyval == GDK_KEY_F1)
		_builder_dialog_help(widget, confopt->conf, confopt->opt);
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
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
	buffer = gtk_entry_buffer_new(argc == 1 ? argv[0] : NULL, -1);
	widget = gtk_entry_new_with_buffer(buffer);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	if(opt->max_input_form > 0)
		gtk_entry_set_max_length(GTK_ENTRY(widget),
				opt->max_input_form);
	if(conf->form.securech != '\0')
		gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
	if(cols > 0)
		gtk_entry_set_width_chars(GTK_ENTRY(widget), cols);
	gtk_widget_show(widget);
	gtk_container_add(GTK_CONTAINER(container), widget);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			dprintf(opt->output_fd, "%s\n",
					gtk_entry_buffer_get_text(buffer));
			break;
	}
	g_object_unref(buffer);
	return ret;
}


/* builder_menu */
static GtkTreeIter * _menu_get_parent(GtkTreeModel * model,
		GtkTreeIter * iter, int depth);
static void _menu_on_row_activated(gpointer data);

int builder_menu(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * window;
	GtkWidget * widget;
	GtkTreeStore * store;
	GtkTreeIter iter, parent, * pparent = NULL;
	GtkTreeViewColumn * column;
	GtkTreeSelection * treesel;
	int i, j, k, n, depth = 0;
	char const * prefix = NULL, * name, * desc, * tooltip;

	j = opt->item_bottomdesc ? 3 : 2;
	if(opt->item_prefix)
		j++;
	if(opt->item_depth)
		j++;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %d, %d (%d), \"%s\")\n", __func__,
			rows, cols, argc, (argc - 1) / j,
			(argv[0] != NULL) ? argv[0] : "(null)");
#endif
	if(argc < 1 || (n = strtol(argv[0], NULL, 10)) < 0
			|| ((argc - 1) % j) != 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	else if(n == 0)
		n = (argc - 1) / j;
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
	store = gtk_tree_store_new(MTS_COUNT, G_TYPE_STRING, G_TYPE_INT,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	if(conf->shadow == false)
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(window),
				GTK_SHADOW_NONE);
	widget = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(widget), GTK_TREE_MODEL(store));
	if(opt->item_bottomdesc)
		gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(widget),
				MTS_TOOLTIP);
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_BROWSE);
	for(i = 0; (i + 1) * j < argc; i++)
	{
		k = i * j + 1;
		if(opt->item_prefix)
			prefix = argv[k++];
		if(opt->item_depth)
		{
			depth = strtol(argv[k++], NULL, 10);
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() depth=%d\n", __func__,
					depth);
#endif
			pparent = _menu_get_parent(GTK_TREE_MODEL(store),
					&parent, depth);
		}
		name = argv[k++];
		desc = argv[k++];
		tooltip = (k < j) ? argv[k++] : NULL;
		gtk_tree_store_append(store, &iter, pparent);
		gtk_tree_store_set(store, &iter, MTS_PREFIX, prefix,
				MTS_DEPTH, depth,
				MTS_NAME, name, MTS_DESCRIPTION, desc,
				(tooltip != NULL) ? MTS_TOOLTIP : -1,
				(tooltip != NULL) ? tooltip : NULL, -1);
		if(opt->item_default != NULL
				&& strcmp(name, opt->item_default) == 0)
			gtk_tree_selection_select_iter(treesel, &iter);
	}
	if(opt->item_prefix == true)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(),
				"text", MTS_PREFIX, NULL);
		gtk_tree_view_column_set_expand(column, FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	if(conf->menu.no_name == false)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(), "text", MTS_NAME,
				NULL);
		gtk_tree_view_column_set_expand(column, FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	if(conf->menu.no_desc == false)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(), "text",
				MTS_DESCRIPTION, NULL);
		gtk_tree_view_column_set_expand(column, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	if(opt->item_depth)
		gtk_tree_view_expand_all(GTK_TREE_VIEW(widget));
	g_signal_connect_swapped(widget, "row-activated",
			G_CALLBACK(_menu_on_row_activated), dialog);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 0);
	gtk_widget_show_all(window);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	switch(ret)
	{
		case BSDDIALOG_HELP:
			_builder_dialog_menu_output(opt, treesel,
					GTK_TREE_MODEL(store), MTS_NAME,
					"HELP ");
			break;
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			_builder_dialog_menu_output(opt, treesel,
					GTK_TREE_MODEL(store), MTS_NAME, NULL);
			break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}

static GtkTreeIter * _menu_get_parent(GtkTreeModel * model,
		GtkTreeIter * parent, int depth)
{
	GtkTreeIter * ret = NULL;
	GtkTreeIter iter;
	gboolean b;
	int d;

	if(depth <= 0)
		return NULL;
	for(b = gtk_tree_model_get_iter_first(model, &iter); b == TRUE;
			b = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, MTS_DEPTH, &d, -1);
		if(d < depth)
		{
			*parent = iter;
			ret = parent;
		}
	}
	return ret;
}

static void _menu_on_row_activated(gpointer data)
{
	GtkWidget * dialog = data;

	gtk_window_activate_default(GTK_WINDOW(dialog));
}


/* builder_mixedform */
static void _mixedform_foreach_buffer(gpointer e, gpointer data);

int builder_mixedform(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * box;
	GtkWidget * widget;
	GSList * l = NULL;
	GtkEntryBuffer * buffer;
	GtkSizeGroup * group;
	int i, k, n, fieldlen, maxletters, flag;
	const int j = 9;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, argc);
#endif
	if(argc < 1 || (n = strtol(argv[0], NULL, 10)) < 0
			|| ((argc - 1) % j) != 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	group = gtk_size_group_new(GTK_SIZE_GROUP_BOTH);
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
	gtk_box_set_spacing(GTK_BOX(container), BORDER_WIDTH);
	for(i = 0; (i + 1) * j < argc; i++)
	{
		k = i * j + 1;
#if GTK_CHECK_VERSION(3, 0, 0)
		box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, BORDER_WIDTH);
#else
		box = gtk_hbox_new(FALSE, BORDER_WIDTH);
#endif
		gtk_container_add(GTK_CONTAINER(container), box);
		/* label */
		widget = gtk_label_new(argv[k]);
#if GTK_CHECK_VERSION(3, 14, 0)
		gtk_widget_set_halign(widget, opt->halign);
#else
		gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
#ifdef WITH_XDIALOG
		gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
#endif
		gtk_size_group_add_widget(group, widget);
		gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
		/* entry */
		buffer = gtk_entry_buffer_new(argv[k + 3], -1);
		l = g_slist_append(l, buffer);
		fieldlen = strtol(argv[k + 6], NULL, 10);
		maxletters = strtol(argv[k + 7], NULL, 10);
		flag = strtol(argv[k + 8], NULL, 10);
		/* XXX do not create an entry if irrelevant */
		if(flag & 0x2 && strcmp(argv[k], argv[k + 3]) == 0)
			continue;
		widget = gtk_entry_new_with_buffer(buffer);
		if(conf->button.always_active == true)
			gtk_entry_set_activates_default(GTK_ENTRY(widget),
					TRUE);
		if(conf->form.securech != '\0')
			gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
		if(fieldlen != 0)
			gtk_entry_set_width_chars(GTK_ENTRY(widget),
					abs(fieldlen));
		if(fieldlen < 0)
			gtk_editable_set_editable(GTK_EDITABLE(widget), FALSE);
		if(maxletters == 0)
			maxletters = abs(fieldlen);
		if(maxletters > 0)
			gtk_entry_set_max_length(GTK_ENTRY(widget), maxletters);
		if(flag & 0x1)
			gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
		if(flag & 0x2)
			gtk_editable_set_editable(GTK_EDITABLE(widget), FALSE);
		gtk_box_pack_start(GTK_BOX(box), widget,
				(maxletters != 0) ? FALSE : TRUE, TRUE, 0);
	}
	gtk_widget_show_all(container);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			g_slist_foreach(l, _mixedform_foreach_buffer,
					(void *)opt);
			break;
	}
	g_slist_foreach(l, (GFunc)g_object_unref, NULL);
	g_slist_free(l);
	return ret;
}

static void _mixedform_foreach_buffer(gpointer e, gpointer data)
{
	GtkEntryBuffer * buffer = e;
	struct options const * opt = data;

	dprintf(opt->output_fd, "%s\n", gtk_entry_buffer_get_text(buffer));
}


/* builder_mixedgauge */
static void _mixedgauge_set_percentage(GtkWidget * widget, int perc);

int builder_mixedgauge(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * box;
	GtkWidget * widget;
	int i, perc;
	const int j = 2;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, argc);
#endif
	if(argc > 1 && (argc % j) == 0)
	{
		error_args(opt->name, argc - 1, argv + 1);
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, opt, NULL, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
	/* items */
	for(i = 0; (i + 1) * j < argc; i++)
	{
#if GTK_CHECK_VERSION(3, 0, 0)
		box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, BORDER_WIDTH);
#else
		box = gtk_hbox_new(FALSE, BORDER_WIDTH);
#endif
		widget = gtk_label_new(argv[i * j + 1]);
		gtk_label_set_single_line_mode(GTK_LABEL(widget), TRUE);
#if GTK_CHECK_VERSION(3, 14, 0)
		gtk_widget_set_halign(widget, opt->halign);
#else
		gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
#ifdef WITH_XDIALOG
		gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
#endif
		gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
		widget = gtk_progress_bar_new();
#if GTK_CHECK_VERSION(3, 0, 0)
		gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(widget), TRUE);
#endif
		_mixedgauge_set_percentage(widget,
				strtol(argv[i * j + 2], NULL, 10));
		gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
		gtk_container_add(GTK_CONTAINER(container), box);
	}
	/* text */
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
#if GTK_CHECK_VERSION(3, 14, 0)
		gtk_widget_set_halign(widget, opt->halign);
#else
		gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
#ifdef WITH_XDIALOG
		gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
#endif
		gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE,
				BORDER_WIDTH);
	}
	/* global progress bar */
	widget = gtk_progress_bar_new();
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(widget), TRUE);
#endif
	perc = (argc >= 1) ? strtol(argv[0], NULL, 10) : 0;
	_mixedgauge_set_percentage(widget, perc);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE, 0);
	gtk_widget_show_all(container);
	ret = _builder_dialog_run(conf, dialog);
	gtk_widget_destroy(dialog);
	return ret;
}

static void _mixedgauge_set_percentage(GtkWidget * widget, int perc)
{
	gdouble fraction = 0.0;
	char buf[16] = "";

	if(perc < 0)
		switch(-perc)
		{
			case 1:
				strcpy(buf, "Success");
				fraction = 1.0;
				break;
			case 2:
				strcpy(buf, "Failed");
				break;
			case 3:
				strcpy(buf, "Passed");
				fraction = 1.0;
				break;
			case 4:
				strcpy(buf, "Completed");
				fraction = 1.0;
				break;
			case 5:
				strcpy(buf, "Checked");
				fraction = 1.0;
				break;
			case 6:
				strcpy(buf, "Done");
				fraction = 1.0;
				break;
			case 7:
				strcpy(buf, "Skipped");
				break;
			case 8:
				strcpy(buf, "In Progress");
				fraction = -1.0;
				break;
			case 9: /* (blank) */
				buf[0] = '\0';
				break;
			case 10:
				strcpy(buf, "N/A");
				break;
			case 11:
				strcpy(buf, "Pending");
				break;
			default:
				strcpy(buf, "Unknown");
				break;
		}
	else
	{
		perc = MIN(perc, 100);
		fraction = (gdouble)perc / 100.0;
		snprintf(buf, sizeof(buf), "%d %%", perc);
	}
	if(fraction < 0.0 || fraction > 1.0)
		/* FIXME keep pulsing until closing */
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(widget));
	else
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget),
				fraction);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), buf);
}


/* builder_msgbox */
static void _msgbox_dialog_buttons(GtkWidget * dialog,
		struct bsddialog_conf const * conf,
		struct options const * opt);

int builder_msgbox(struct bsddialog_conf const * conf,
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
	dialog = _builder_dialog(conf, opt, text, rows, cols);
	_msgbox_dialog_buttons(dialog, conf, NULL);
	ret = _builder_dialog_run(conf, dialog);
	gtk_widget_destroy(dialog);
	return ret;
}

static void _msgbox_dialog_buttons(GtkWidget * dialog,
		struct bsddialog_conf const * conf,
		struct options const * opt)
{
	char const * label;
#ifndef WITH_XDIALOG
	(void) opt;
#endif

	if(conf->button.without_cancel != true
			&& conf->button.cancel_label != NULL)
	{
		label = (conf->button.cancel_label != NULL)
			? conf->button.cancel_label : "Cancel";
		gtk_dialog_add_button(GTK_DIALOG(dialog), label,
				GTK_RESPONSE_CANCEL);
	}
	if(conf->button.with_extra == true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.extra_label != NULL)
				? conf->button.extra_label : "Extra",
				BSDDIALOG_EXTRA);
	if(conf->button.without_ok != true)
	{
		label = (conf->button.ok_label != NULL)
			? conf->button.ok_label : "OK";
#ifdef WITH_XDIALOG
		if(opt != NULL && opt->wizard)
			label = "Next";
#endif
		gtk_dialog_add_button(GTK_DIALOG(dialog), label,
				GTK_RESPONSE_OK);
	}
	if(conf->button.with_help == true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.help_label != NULL)
				? conf->button.help_label : "Help",
				GTK_RESPONSE_HELP);
#ifdef WITH_XDIALOG
	if(opt != NULL && opt->wizard)
		gtk_dialog_add_button(GTK_DIALOG(dialog), "Previous", 3);
#endif
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
			(conf->button.without_cancel != true
			 && conf->button.cancel_label != NULL
			 && conf->button.default_cancel)
			? GTK_RESPONSE_CANCEL : GTK_RESPONSE_OK);
}


/* builder_passwordbox */
static void _passwordbox_on_toggled(GtkWidget * widget, gpointer data);

int builder_passwordbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * checkbox;
	GtkWidget * widget;
	GtkEntryBuffer * buffer;

	if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
	buffer = gtk_entry_buffer_new(NULL, -1);
	widget = gtk_entry_new_with_buffer(buffer);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	if(opt->max_input_form > 0)
		gtk_entry_set_max_length(GTK_ENTRY(widget),
				opt->max_input_form);
	gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
	if(cols > 0)
		gtk_entry_set_width_chars(GTK_ENTRY(widget), cols);
	gtk_box_pack_start(GTK_BOX(container), widget, FALSE, TRUE,
			BORDER_WIDTH);
	checkbox = gtk_check_button_new_with_label("Show password");
	g_signal_connect(checkbox, "toggled",
			G_CALLBACK(_passwordbox_on_toggled), widget);
	gtk_container_add(GTK_CONTAINER(container), checkbox);
	gtk_widget_show_all(container);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			dprintf(opt->output_fd, "%s\n",
					gtk_entry_buffer_get_text(buffer));
			break;
	}
	g_object_unref(buffer);
	return ret;
}

static void _passwordbox_on_toggled(GtkWidget * widget, gpointer data)
{
	GtkWidget * entry = data;
	gboolean active;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_entry_set_visibility(GTK_ENTRY(entry), active);
}


/* builder_passwordform */
int builder_passwordform(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	struct bsddialog_conf conf2 = *conf;

	/* XXX hack */
	conf2.form.securech = '*';
	return builder_form(&conf2, text, rows, cols, argc, argv, opt);
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

	if(argc <= 0)
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
	pd.dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(pd.dialog));
#else
	container = pd.dialog->vbox;
#endif
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
	_builder_dialog_buttons(pd.dialog, conf, opt);
	ret = _builder_dialog_run(conf, pd.dialog);
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
	snprintf(buf, sizeof(buf), "%u", (pd->secs > 0) ? --pd->secs : 0);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pd->widget), buf);
	return TRUE;
}


/* builder_radiolist */
static gboolean _radiolist_foreach_response(GtkTreeModel * model,
		GtkTreePath * path, GtkTreeIter * iter, gpointer data);
static GtkTreeIter * _radiolist_get_parent(GtkTreeModel * model,
		GtkTreeIter * iter, int depth, char const * name);
static gboolean _radiolist_get_parent_foreach(GtkTreeModel * model,
		GtkTreePath * path, GtkTreeIter * iter, gpointer data);
static void _radiolist_on_row_activated(GtkWidget * widget, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data);
static void _radiolist_on_row_toggled(GtkCellRenderer * renderer, char * path,
		gpointer data);
static gboolean _radiolist_on_row_toggled_foreach(GtkTreeModel * model,
		GtkTreePath * path, GtkTreeIter * iter, gpointer data);

int builder_radiolist(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * window;
	GtkWidget * widget;
	GtkTreeStore * store;
	GtkTreeIter iter, parent, * pparent = NULL;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	GtkTreeSelection * treesel;
	int i, j, k, n, depth = 0;
	gboolean set;
	char const * prefix = NULL, * name, * desc, * tooltip;

	j = opt->item_bottomdesc ? 4 : 3;
	if(opt->item_prefix)
		j++;
	if(opt->item_depth)
		j++;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %d, %d (%d), \"%s\")\n", __func__,
			rows, cols, argc, (argc - 1) / j,
			(argv[0] != NULL) ? argv[0] : "(null)");
#endif
	if(argc < 1 || (n = strtol(argv[0], NULL, 10)) < 0
			|| ((argc - 1) % j) != 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	else if(n == 0)
		n = (argc - 1) / j;
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
	store = gtk_tree_store_new(RTS_COUNT, G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING);
	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	if(conf->shadow == false)
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(window),
				GTK_SHADOW_NONE);
	widget = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(widget), GTK_TREE_MODEL(store));
	if(opt->item_bottomdesc)
		gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(widget),
				RTS_TOOLTIP);
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_BROWSE);
	for(i = 0, set = FALSE; (i + 1) * j < argc; i++)
	{
		k = i * j + 1;
		if(opt->item_prefix)
			prefix = argv[k++];
		if(opt->item_depth)
		{
			depth = strtol(argv[k++], NULL, 10);
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() depth=%d\n", __func__,
					depth);
#endif
			pparent = _radiolist_get_parent(GTK_TREE_MODEL(store),
					&parent, depth, argv[k]);
		}
		name = argv[k++];
		desc = argv[k++];
		set = (set == FALSE && strcasecmp(argv[k], "on") == 0)
			? TRUE : FALSE;
		tooltip = (++k < j) ? argv[k] : NULL;
		gtk_tree_store_append(store, &iter, pparent);
		gtk_tree_store_set(store, &iter, RTS_PREFIX, prefix,
				RTS_SET, set, RTS_DEPTH, depth,
				RTS_NAME, name, RTS_DESCRIPTION, desc,
				(tooltip != NULL) ? RTS_TOOLTIP : -1,
				(tooltip != NULL) ? tooltip : NULL, -1);
		if(opt->item_default != NULL
				&& strcmp(name, opt->item_default) == 0)
			gtk_tree_selection_select_iter(treesel, &iter);
	}
	if(opt->item_prefix == true)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(),
				"text", RTS_PREFIX, NULL);
		gtk_tree_view_column_set_expand(column, FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	renderer = gtk_cell_renderer_toggle_new();
	gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(renderer),
			TRUE);
	g_signal_connect(renderer, "toggled",
			G_CALLBACK(_radiolist_on_row_toggled), store);
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
			"active", RTS_SET, NULL);
	gtk_tree_view_column_set_expand(column, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	if(conf->menu.no_name == false)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(), "text", RTS_NAME,
				NULL);
		gtk_tree_view_column_set_expand(column, FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	if(conf->menu.no_desc == false)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(), "text",
				RTS_DESCRIPTION, NULL);
		gtk_tree_view_column_set_expand(column, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	if(opt->item_depth)
		gtk_tree_view_expand_all(GTK_TREE_VIEW(widget));
	g_signal_connect(widget, "row-activated",
			G_CALLBACK(_radiolist_on_row_activated), NULL);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 0);
	gtk_widget_show_all(window);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	switch(ret)
	{
		case BSDDIALOG_HELP:
			_builder_dialog_menu_output(opt, treesel,
					GTK_TREE_MODEL(store), RTS_NAME,
					"HELP ");
			break;
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			gtk_tree_model_foreach(GTK_TREE_MODEL(store),
					_radiolist_foreach_response,
					(gpointer)opt);
			break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}

static gboolean _radiolist_foreach_response(GtkTreeModel * model,
		GtkTreePath * path, GtkTreeIter * iter, gpointer data)
{
	struct options const * opt = data;
	gboolean set;
	gchar * p;
	gboolean toquote;
	char quotech;
	(void) path;

	gtk_tree_model_get(model, iter, RTS_SET, &set, -1);
	if(set == FALSE)
		return FALSE;
	gtk_tree_model_get(model, iter, RTS_NAME, &p, -1);
	toquote = FALSE;
	if(string_needs_quoting(p))
		toquote = opt->item_always_quote;
	if(toquote)
	{
		quotech = opt->item_singlequote ? '\'' : '"';
		dprintf(opt->output_fd, "%c%s%c\n", quotech, p, quotech);
	}
	else
		dprintf(opt->output_fd, "%s\n", p);
	g_free(p);
	return TRUE;
}

static GtkTreeIter * _radiolist_get_parent(GtkTreeModel * model,
		GtkTreeIter * parent, int depth, char const * name)
{
	struct radiolist_data rd;

	if(depth <= 0)
		return NULL;
	rd.depth = depth;
	rd.parent = parent;
	rd.pparent = NULL;
	rd.name = name;
	gtk_tree_model_foreach(model, _radiolist_get_parent_foreach, &rd);
	return rd.pparent;
}

static gboolean _radiolist_get_parent_foreach(GtkTreeModel * model,
		GtkTreePath * path, GtkTreeIter * iter, gpointer data)
{
	struct radiolist_data * rd = data;
	int d;
	(void) path;

	gtk_tree_model_get(model, iter, RTS_DEPTH, &d, -1);
	if(d < rd->depth)
	{
		*(rd->parent) = *iter;
		rd->pparent = rd->parent;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s(\"%s\") %d < %d\n", __func__,
				rd->name, d, rd->depth);
#endif
	}
	return FALSE;
}

static void _radiolist_on_row_activated(GtkWidget * widget, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data)
{
	GtkTreeModel * model;
	GtkTreeIter iter;
	(void) column;
	(void) data;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	gtk_tree_model_foreach(model, _radiolist_on_row_toggled_foreach, NULL);
	if(gtk_tree_model_get_iter(model, &iter, path) == FALSE)
		return;
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter, RTS_SET, TRUE, -1);
}

static void _radiolist_on_row_toggled(GtkCellRenderer * renderer, char * path,
		gpointer data)
{
	GtkTreeStore * store = data;
	GtkTreePath * tp;
	GtkTreeIter iter;
	gboolean b;
	(void) renderer;

	if((tp = gtk_tree_path_new_from_string(path)) == NULL)
		return;
	gtk_tree_model_foreach(GTK_TREE_MODEL(store),
			_radiolist_on_row_toggled_foreach, NULL);
	b = gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, tp);
	gtk_tree_path_free(tp);
	if(b == FALSE)
		return;
	gtk_tree_store_set(store, &iter, RTS_SET, TRUE, -1);
}

static gboolean _radiolist_on_row_toggled_foreach(GtkTreeModel * model,
		GtkTreePath * path, GtkTreeIter * iter, gpointer data)
{
	gboolean set;
	(void) path;
	(void) data;

	gtk_tree_model_get(model, iter, RTS_SET, &set, -1);
	if(set)
	{
		/* XXX assumes only one row is set */
		gtk_tree_store_set(GTK_TREE_STORE(model), iter, RTS_SET, FALSE,
				-1);
		return TRUE;
	}
	return FALSE;
}


/* builder_rangebox */
int builder_rangebox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * box;
	GtkWidget * widget;
	int value = 0, min, max;

	if(argc == 3)
		/* XXX detect and report errors */
		value = strtol(argv[2], NULL, 10);
	else if(argc < 2 || argc > 3)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	min = strtol(argv[0], NULL, 10);
	max = strtol(argv[1], NULL, 10);
	if(min > max)
	{
		printf("Error: min > max\n");
		exit(EXITCODE(BSDDIALOG_ERROR));
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	box = dialog->vbox;
#endif
#if GTK_CHECK_VERSION(3, 0, 0)
	widget = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
			(gdouble)min, (gdouble)max, 1.0);
#else
	widget = gtk_hscale_new_with_range((gdouble)min, (gdouble)max, 1.0);
#endif
	if(argc == 3)
		gtk_range_set_value(GTK_RANGE(widget), (gdouble)value);
	gtk_widget_show(widget);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, BORDER_WIDTH);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	value = gtk_range_get_value(GTK_RANGE(widget));
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			dprintf(opt->output_fd, "%d\n", value);
			break;
	}
	return ret;
}


/* builder_textbox */
static gboolean _textbox_on_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data);
static gboolean _textbox_on_can_read_eof(gpointer data);
#ifdef WITH_XDIALOG
static gboolean _textbox_on_can_read_scroll(gpointer data);
#endif
static gboolean _textbox_on_idle(gpointer data);
#if GTK_CHECK_VERSION(2, 10, 0)
# ifdef WITH_XDIALOG
static void _textbox_on_print(gpointer data);
# endif
#endif

int builder_textbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	struct textbox_data td;
	GtkWidget * container;
	GtkWidget * window;
#ifdef WITH_XDIALOG
	PangoFontDescription * desc = NULL;
#endif

	if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	td.opt = opt;
	td.editable = FALSE;
#ifdef WITH_XDIALOG
	td.scroll = FALSE;
	td.button = NULL;
#endif
	td.filename = text;
	td.dialog = _builder_dialog(conf, opt, NULL, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(td.dialog));
#else
	container = td.dialog->vbox;
#endif
	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	if(conf->shadow == false)
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(window),
				GTK_SHADOW_NONE);
	td.view = gtk_text_view_new();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(td.view), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(td.view), td.editable);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(td.view), GTK_WRAP_WORD_CHAR);
#ifdef WITH_XDIALOG
	if(opt->fixed_font)
	{
		desc = pango_font_description_from_string("Monospace");
# if GTK_CHECK_VERSION(3, 0, 0)
		gtk_widget_override_font(td.view, desc);
# else
		gtk_widget_modify_font(td.view, desc);
# endif
	}
#endif
	td.buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(td.view));
	gtk_container_add(GTK_CONTAINER(window), td.view);
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 0);
	gtk_widget_show_all(window);
#ifdef WITH_XDIALOG
	if(!opt->without_buttons)
#endif
	{
#if GTK_CHECK_VERSION(2, 10, 0)
# ifdef WITH_XDIALOG
		if(opt->print != NULL)
		{
#  if GTK_CHECK_VERSION(3, 12, 0)
			container = gtk_dialog_get_header_bar(
					GTK_DIALOG(td.dialog));
			if(container == NULL)
#  endif
#  if GTK_CHECK_VERSION(2, 14, 0)
				container = gtk_dialog_get_action_area(
						GTK_DIALOG(td.dialog));
#  else
			container = td.dialog->action_area;
#  endif
#  if GTK_CHECK_VERSION(3, 10, 0)
			td.button = gtk_button_new_with_label("Print");
#  else
			td.button = gtk_button_new_from_stock(GTK_STOCK_PRINT);
#  endif
			gtk_widget_set_sensitive(td.button, FALSE);
			g_signal_connect_swapped(td.button, "clicked",
					G_CALLBACK(_textbox_on_print), &td);
			gtk_widget_show(td.button);
			gtk_container_add(GTK_CONTAINER(container), td.button);
		}
# endif
#endif
#ifdef WITH_XDIALOG
		if(conf->button.without_cancel != true
				&& opt->high_compat == false)
			gtk_dialog_add_button(GTK_DIALOG(td.dialog), "Cancel",
					GTK_RESPONSE_CANCEL);
#endif
		gtk_dialog_add_button(GTK_DIALOG(td.dialog), "Exit",
				GTK_RESPONSE_OK);
	}
#if GTK_CHECK_VERSION(3, 12, 0)
	if((container = gtk_dialog_get_header_bar(GTK_DIALOG(td.dialog)))
			!= NULL)
		gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(container),
				FALSE);
#endif
	td.id = g_idle_add(_textbox_on_idle, &td);
	ret = _builder_dialog_run(conf, td.dialog);
	if(td.id != 0)
		g_source_remove(td.id);
	gtk_widget_destroy(td.dialog);
#ifdef WITH_XDIALOG
	if(desc != NULL)
		pango_font_description_free(desc);
#endif
	return ret;
}

static gboolean _textbox_on_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data)
{
	struct textbox_data * td = data;
	GIOStatus status;
	char buf[BUFSIZ];
	gsize r;
	GError * error = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(condition != G_IO_IN)
	{
		_builder_dialog_error(td->dialog, NULL, NULL,
				"Unexpected condition");
		return _textbox_on_can_read_eof(td);
	}
	status = g_io_channel_read_chars(channel, buf, sizeof(buf), &r, &error);
	if(status == G_IO_STATUS_ERROR)
	{
		_builder_dialog_error(td->dialog, NULL, NULL, error->message);
		g_error_free(error);
		return _textbox_on_can_read_eof(td);
	}
	else if(status == G_IO_STATUS_AGAIN)
		return TRUE;
	else if(status == G_IO_STATUS_EOF)
		return _textbox_on_can_read_eof(td);
	gtk_text_buffer_insert(td->buffer, &td->iter, buf, r);
#ifdef WITH_XDIALOG
	if(td->scroll)
	{
		td->id = g_idle_add(_textbox_on_can_read_scroll, td);
		return FALSE;
	}
#endif
	return TRUE;
}

static gboolean _textbox_on_can_read_eof(gpointer data)
{
	struct textbox_data * td = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	td->id = 0;
#ifdef WITH_XDIALOG
	if(td->button != NULL)
		gtk_widget_set_sensitive(td->button, TRUE);
#endif
	return FALSE;
}

#ifdef WITH_XDIALOG
static gboolean _textbox_on_can_read_scroll(gpointer data)
{
	struct textbox_data * td = data;

	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(td->view), &td->iter,
			0.0, TRUE, 0.0, 1.0);
	td->id = g_io_add_watch(td->channel, G_IO_IN, _textbox_on_can_read, td);
	return FALSE;
}
#endif

static gboolean _textbox_on_idle(gpointer data)
{
	struct textbox_data * td = data;
	char buf[BUFSIZ];
	gboolean close = TRUE;

#ifdef WITH_XDIALOG
	if(strcmp(td->filename, "-") == 0)
	{
		td->fd = STDIN_FILENO;
		close = FALSE;
	}
	else
#endif
	if((td->fd = open(td->filename, O_RDONLY)) <= -1)
	{
		snprintf(buf, sizeof(buf), "%s: %s", td->filename,
				strerror(errno));
		_builder_dialog_error(td->dialog, NULL, NULL, buf);
		gtk_dialog_response(GTK_DIALOG(td->dialog), BSDDIALOG_ERROR);
		td->id = 0;
#ifdef WITH_XDIALOG
		if(td->button != NULL)
			gtk_widget_set_sensitive(td->button, TRUE);
#endif
		return FALSE;
	}
	td->channel = g_io_channel_unix_new(td->fd);
	g_io_channel_set_close_on_unref(td->channel, close);
	g_io_channel_set_encoding(td->channel, NULL, NULL);
	/* XXX ignore errors */
	g_io_channel_set_flags(td->channel, g_io_channel_get_flags(td->channel)
			| G_IO_FLAG_NONBLOCK, NULL);
	td->id = g_io_add_watch(td->channel, G_IO_IN, _textbox_on_can_read, td);
	gtk_text_buffer_get_start_iter(td->buffer, &td->iter);
	return FALSE;
}

#if GTK_CHECK_VERSION(2, 10, 0)
# ifdef WITH_XDIALOG
static void _print_dialog_on_begin_print(gpointer data);
static void _print_dialog_on_done(GtkPrintOperation * operation,
		GtkPrintOperationResult result, gpointer data);
static void _print_dialog_on_draw_page(GtkPrintOperation * operation,
		GtkPrintContext * context, gint page, gpointer data);
static void _print_dialog_on_end_print(gpointer data);
static gboolean _print_dialog_on_paginate(GtkPrintOperation * operation,
		GtkPrintContext * context, gpointer data);

static void _textbox_on_print(gpointer data)
{
	struct textbox_data * td = data;
	GtkPrintOperation * operation;
	GtkPrintSettings * settings;
	GError * error = NULL;

	operation = gtk_print_operation_new();
	gtk_print_operation_set_embed_page_setup(operation, TRUE);
	gtk_print_operation_set_unit(operation, GTK_UNIT_POINTS);
	gtk_print_operation_set_use_full_page(operation, FALSE);
	g_signal_connect_swapped(operation, "begin-print", G_CALLBACK(
				_print_dialog_on_begin_print), td);
	g_signal_connect(operation, "done", G_CALLBACK(_print_dialog_on_done),
			td);
	g_signal_connect(operation, "draw-page", G_CALLBACK(
				_print_dialog_on_draw_page), td);
	g_signal_connect_swapped(operation, "end-print", G_CALLBACK(
				_print_dialog_on_end_print), td);
	g_signal_connect(operation, "paginate", G_CALLBACK(
				_print_dialog_on_paginate), td);
	settings = gtk_print_settings_new();
	if(td->opt->print != NULL && strlen(td->opt->print) > 0)
		gtk_print_settings_set_printer(settings, td->opt->print);
	gtk_print_operation_set_print_settings(operation, settings);
	gtk_print_operation_run(operation,
			GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
			GTK_WINDOW(td->dialog), &error);
	g_object_unref(settings);
	g_object_unref(operation);
	if(error)
	{
		_builder_dialog_error(td->dialog, NULL, NULL, error->message);
		g_error_free(error);
	}
}

static void _print_dialog_on_begin_print(gpointer data)
{
	const gdouble size = 9.0;
	struct textbox_data * td = data;
	char const * font;

	/* FIXME obtain the actual font */
	font = td->opt->fixed_font ? "Monospace" : "Sans";
	td->font = pango_font_description_from_string(font);
	pango_font_description_set_size(td->font,
			pango_units_from_double(size));
	td->font_size = size;
	td->line_space = 0.0;
}

static void _print_dialog_on_done(GtkPrintOperation * operation,
		GtkPrintOperationResult result, gpointer data)
{
	struct textbox_data * td = data;
	GError * error = NULL;

	switch(result)
	{
		case GTK_PRINT_OPERATION_RESULT_ERROR:
			gtk_print_operation_get_error(operation, &error);
			_builder_dialog_error(td->dialog, NULL, NULL,
					error->message);
			g_error_free(error);
			break;
		default:
			break;
	}
}

static void _print_dialog_on_draw_page(GtkPrintOperation * operation,
		GtkPrintContext * context, gint page, gpointer data)
{
	struct textbox_data * td = data;
	cairo_t * cairo;
	PangoLayout * layout;
	guint i;
	gboolean valid = TRUE;
	GtkTextIter end;
	gchar * p;
	(void) operation;

	cairo = gtk_print_context_get_cairo_context(context);
	layout = gtk_print_context_create_pango_layout(context);
	/* set the font */
	pango_layout_set_font_description(layout, td->font);
	/* print the text */
	cairo_move_to(cairo, 0.0, 0.0);
	gtk_text_buffer_get_iter_at_line(td->buffer, &td->iter,
			td->line_count * page);
	for(i = 0, valid = !gtk_text_iter_is_end(&td->iter);
			i < td->line_count && valid == TRUE;
			i++, valid = gtk_text_iter_forward_line(&td->iter))
	{
		end = td->iter;
		if(!gtk_text_iter_ends_line(&end))
			gtk_text_iter_forward_to_line_end(&end);
		p = gtk_text_buffer_get_text(td->buffer, &td->iter, &end,
				FALSE);
		/* FIXME the line may be too long */
		pango_layout_set_text(layout, p, -1);
		g_free(p);
		pango_cairo_show_layout(cairo, layout);
		cairo_rel_move_to(cairo, 0.0, td->font_size + td->line_space);
	}
	g_object_unref(layout);
}

static void _print_dialog_on_end_print(gpointer data)
{
	struct textbox_data * td = data;

	pango_font_description_free(td->font);
	if(td->editable)
		gtk_text_view_set_editable(GTK_TEXT_VIEW(td->view), TRUE);
}

static gboolean _print_dialog_on_paginate(GtkPrintOperation * operation,
		GtkPrintContext * context, gpointer data)
{
	struct textbox_data * td = data;
	gint count;
	double height;

	/* count the lines to print */
	gtk_text_view_set_editable(GTK_TEXT_VIEW(td->view), FALSE);
	count = gtk_text_buffer_get_line_count(td->buffer);
	/* count the pages required */
	height = gtk_print_context_get_height(context);
	td->line_count = floor(height / (td->font_size + td->line_space));
	gtk_print_operation_set_n_pages(operation,
			((count - 1) / td->line_count) + 1);
	return TRUE;
}
# endif
#endif


/* builder_timebox */
static gboolean _timebox_on_output(GtkWidget * widget);

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
		return _builder_dialog_error(NULL, conf, opt, strerror(errno));
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
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
#if GTK_CHECK_VERSION(3, 0, 0)
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, BORDER_WIDTH);
#else
	box = gtk_hbox_new(FALSE, BORDER_WIDTH);
#endif
	td.hour = gtk_spin_button_new_with_range(0.0, 23.0, 1.0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(td.hour), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(td.hour), (gdouble)hour);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(td.hour), TRUE);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(td.hour), TRUE);
	g_signal_connect(td.hour, "output", G_CALLBACK(_timebox_on_output),
			NULL);
	gtk_box_pack_start(GTK_BOX(box), td.hour, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), gtk_label_new(":"), FALSE, TRUE,
			BORDER_WIDTH);
	td.minute = gtk_spin_button_new_with_range(0.0, 59.0, 1.0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(td.minute), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(td.minute), (gdouble)minute);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(td.minute), TRUE);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(td.minute), TRUE);
	g_signal_connect(td.minute, "output", G_CALLBACK(_timebox_on_output),
			NULL);
	gtk_box_pack_start(GTK_BOX(box), td.minute, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), gtk_label_new(":"), FALSE, TRUE,
			BORDER_WIDTH);
	td.second = gtk_spin_button_new_with_range(0.0, 60.0, 1.0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(td.second), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(td.second), (gdouble)second);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(td.second), TRUE);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(td.second), TRUE);
	g_signal_connect(td.second, "output", G_CALLBACK(_timebox_on_output),
			NULL);
	gtk_box_pack_start(GTK_BOX(box), td.second, TRUE, TRUE, 0);
	gtk_widget_show_all(box);
	gtk_container_add(GTK_CONTAINER(container), box);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
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

static gboolean _timebox_on_output(GtkWidget * widget)
{
	char buf[3];

	snprintf(buf, sizeof(buf), "%02u", gtk_spin_button_get_value_as_int(
				GTK_SPIN_BUTTON(widget)));
	gtk_entry_set_text(GTK_ENTRY(widget), buf);
	return TRUE;
}


/* builder_treeview */
int builder_treeview(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * window;
	GtkWidget * widget;
	GtkTreeStore * store;
	GtkTreeIter iter, parent, * pparent = NULL;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	GtkTreeSelection * treesel;
	int i, j, k, n, depth = 0;
	gboolean set;
	char const * prefix = NULL, * name, * desc, * tooltip;

	j = opt->item_bottomdesc ? 5 : 4;
	if(opt->item_prefix)
		j++;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %d, %d (%d), \"%s\")\n", __func__,
			rows, cols, argc, (argc - 1) / j,
			(argv[0] != NULL) ? argv[0] : "(null)");
#endif
	if(argc < 1 || (n = strtol(argv[0], NULL, 10)) < 0
			|| ((argc - 1) % j) != 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	else if(n == 0)
		n = (argc - 1) / j;
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
	store = gtk_tree_store_new(RTS_COUNT, G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING);
	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	if(conf->shadow == false)
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(window),
				GTK_SHADOW_NONE);
	widget = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(widget), GTK_TREE_MODEL(store));
	if(opt->item_bottomdesc)
		gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(widget),
				RTS_TOOLTIP);
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_BROWSE);
	for(i = 0, set = FALSE; (i + 1) * j < argc; i++)
	{
		k = i * j + 1;
		if(opt->item_prefix)
			prefix = argv[k++];
		name = argv[k++];
		desc = argv[k++];
		set = (set == FALSE && strcasecmp(argv[k], "on") == 0)
			? TRUE : FALSE;
		depth = strtol(argv[++k], NULL, 10);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() depth=%d\n", __func__, depth);
#endif
		pparent = _radiolist_get_parent(GTK_TREE_MODEL(store), &parent,
				depth, name);
		tooltip = (++k < j) ? argv[k] : NULL;
		gtk_tree_store_append(store, &iter, pparent);
		gtk_tree_store_set(store, &iter, RTS_PREFIX, prefix,
				RTS_SET, set, RTS_DEPTH, depth,
				RTS_NAME, name, RTS_DESCRIPTION, desc,
				(tooltip != NULL) ? RTS_TOOLTIP : -1,
				(tooltip != NULL) ? tooltip : NULL, -1);
		if(opt->item_default != NULL
				&& strcmp(name, opt->item_default) == 0)
			gtk_tree_selection_select_iter(treesel, &iter);
	}
	if(opt->item_prefix == true)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(),
				"text", RTS_PREFIX, NULL);
		gtk_tree_view_column_set_expand(column, FALSE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	renderer = gtk_cell_renderer_toggle_new();
	gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(renderer),
			TRUE);
	g_signal_connect(renderer, "toggled",
			G_CALLBACK(_radiolist_on_row_toggled), store);
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
			"active", RTS_SET, NULL);
	gtk_tree_view_column_set_expand(column, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	if(conf->menu.no_desc == false)
	{
		column = gtk_tree_view_column_new_with_attributes(NULL,
				gtk_cell_renderer_text_new(), "text",
				RTS_DESCRIPTION, NULL);
		gtk_tree_view_column_set_expand(column, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	}
	gtk_tree_view_expand_all(GTK_TREE_VIEW(widget));
	g_signal_connect(widget, "row-activated",
			G_CALLBACK(_radiolist_on_row_activated), NULL);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 0);
	gtk_widget_show_all(window);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	switch(ret)
	{
		case BSDDIALOG_HELP:
			_builder_dialog_menu_output(opt, treesel,
					GTK_TREE_MODEL(store), RTS_NAME,
					"HELP ");
			break;
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			gtk_tree_model_foreach(GTK_TREE_MODEL(store),
					_radiolist_foreach_response,
					(gpointer)opt);
			break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}


/* builder_yesno */
int builder_yesno(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	char const * label;

	if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, opt, text, rows, cols);
	if(conf->button.without_ok != true)
	{
		label = (conf->button.ok_label != NULL)
			? conf->button.ok_label : "Yes";
#ifdef WITH_XDIALOG
		if(opt->wizard)
			label = "Next";
#endif
		gtk_dialog_add_button(GTK_DIALOG(dialog), label,
				GTK_RESPONSE_YES);
	}
	if(conf->button.without_cancel != true)
	{
		label = (conf->button.cancel_label != NULL)
			? conf->button.cancel_label : "No";
#ifdef WITH_XDIALOG
		if(opt->wizard)
			label = "Cancel";
#endif
		gtk_dialog_add_button(GTK_DIALOG(dialog), label,
				GTK_RESPONSE_NO);
	}
	if(conf->button.with_extra == true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.extra_label != NULL)
				? conf->button.extra_label : "Extra",
				BSDDIALOG_EXTRA);
	if(conf->button.with_help == true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.help_label != NULL)
				? conf->button.help_label : "Help",
				GTK_RESPONSE_HELP);
#ifdef WITH_XDIALOG
	if(opt->wizard)
		gtk_dialog_add_button(GTK_DIALOG(dialog), "Previous", 3);
#endif
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
			conf->button.default_cancel
			? GTK_RESPONSE_NO : GTK_RESPONSE_YES);
	ret = _builder_dialog_run(conf, dialog);
	gtk_widget_destroy(dialog);
	return ret;
}


/* builder_dialog */
static gboolean _dialog_on_key_press(GtkWidget * widget, GdkEventKey * event,
		gpointer data);

static GtkWidget * _builder_dialog(struct bsddialog_conf const * conf,
		struct options const * opt, char const * text,
		int rows, int cols)
{
	GtkWidget * dialog;
#if GTK_CHECK_VERSION(3, 12, 0)
	const GtkDialogFlags flags = GTK_DIALOG_USE_HEADER_BAR;
#else
	const GtkDialogFlags flags = 0;
#endif
	GtkWidget * container;
	GtkWidget * box;
	GtkWidget * widget;
	gdouble ex;
	GdkRectangle workarea;
	struct confopt_data confopt = { conf, opt };

	dialog = gtk_dialog_new_with_buttons(conf->title, NULL, flags, NULL,
			NULL);
	if(conf->key.enable_esc == false)
	{
		gtk_window_set_deletable(GTK_WINDOW(dialog), FALSE);
#if GTK_CHECK_VERSION(3, 12, 0)
		if((widget = gtk_dialog_get_header_bar(GTK_DIALOG(dialog)))
				!= NULL)
			gtk_header_bar_set_show_close_button(
					GTK_HEADER_BAR(widget), FALSE);
#endif
	}
	if(conf->key.f1_file != NULL || conf->key.f1_message != NULL)
		g_signal_connect(dialog, "key-press-event",
				G_CALLBACK(_dialog_on_key_press), &confopt);
#if GTK_CHECK_VERSION(3, 12, 0)
	if(conf->bottomtitle != NULL
			&& (widget = gtk_dialog_get_header_bar(
					GTK_DIALOG(dialog))) != NULL)
		gtk_header_bar_set_subtitle(GTK_HEADER_BAR(widget),
				conf->bottomtitle);
#endif
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
	/* XXX gdk_screen_get_default() may fail */
	ex = get_font_size(gdk_screen_get_default());
	get_workarea(gdk_screen_get_default(), &workarea);
	if(rows == BSDDIALOG_AUTOSIZE)
		rows = (int)(workarea.height / ex / 2) - 9;
	if(cols == BSDDIALOG_AUTOSIZE)
		cols = (int)(workarea.width / ex) - 4;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() ex=%f cols=%d rows=%d\n", __func__,
			ex, cols, rows);
#endif
#ifdef WITH_XDIALOG
	if(opt->pixelsize)
		gtk_widget_set_size_request(container, cols, rows);
	else
#endif
		gtk_widget_set_size_request(container, cols * ex,
				rows * ex * 2);
	if(text != NULL)
	{
#if GTK_CHECK_VERSION(3, 0, 0)
		box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
#else
		box = gtk_hbox_new(FALSE, 0);
#endif
#ifdef WITH_XDIALOG
		if(opt->icon != NULL)
		{
			widget = gtk_image_new_from_file(opt->icon);
			gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE,
					0);
			gtk_widget_show(widget);
		}
#endif
		widget = gtk_label_new(text);
		gtk_label_set_line_wrap(GTK_LABEL(widget), TRUE);
		gtk_label_set_line_wrap_mode(GTK_LABEL(widget),
				PANGO_WRAP_WORD_CHAR);
		gtk_label_set_single_line_mode(GTK_LABEL(widget), FALSE);
#if GTK_CHECK_VERSION(3, 10, 0)
		if(rows > 0)
			gtk_label_set_lines(GTK_LABEL(widget), rows);
#endif
#if GTK_CHECK_VERSION(3, 14, 0)
		gtk_widget_set_halign(widget, opt->halign);
#else
		gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
#ifdef WITH_XDIALOG
		gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
#endif
		gtk_widget_show(widget);
		gtk_widget_show(box);
		gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE,
				BORDER_WIDTH);
		gtk_box_pack_start(GTK_BOX(container), box, FALSE, TRUE, 0);
	}
	if(conf->x == BSDDIALOG_FULLSCREEN || conf->y == BSDDIALOG_FULLSCREEN)
		gtk_window_fullscreen(GTK_WINDOW(dialog));
	else if(conf->x != BSDDIALOG_AUTOSIZE && conf->y != BSDDIALOG_AUTOSIZE)
		gtk_window_move(GTK_WINDOW(dialog), conf->x, conf->y);
	else
		gtk_window_set_position(GTK_WINDOW(dialog), opt->position);
#ifdef WITH_XDIALOG
	if(opt->wmclass != NULL)
		gtk_window_set_wmclass(GTK_WINDOW(dialog), opt->wmclass,
				opt->wmclass);
#endif
	return dialog;
}

static gboolean _dialog_on_key_press(GtkWidget * widget, GdkEventKey * event,
		gpointer data)
{
	struct confopt_data * confopt = data;

	if(event->keyval == GDK_KEY_F1)
		_builder_dialog_help(widget, confopt->conf, confopt->opt);
	return FALSE;
}


/* builder_dialog_buttons */
static void _builder_dialog_buttons(GtkWidget * dialog,
		struct bsddialog_conf const * conf,
		struct options const * opt)
{
	char const * label;
#ifndef WITH_XDIALOG
	(void) opt;
#endif

	if(conf->button.without_cancel != true)
	{
		label = (conf->button.cancel_label != NULL)
			? conf->button.cancel_label : "Cancel";
		gtk_dialog_add_button(GTK_DIALOG(dialog), label,
				GTK_RESPONSE_CANCEL);
	}
	if(conf->button.with_extra == true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.extra_label != NULL)
				? conf->button.extra_label : "Extra",
				BSDDIALOG_EXTRA);
	if(conf->button.without_ok != true)
	{
		label = (conf->button.ok_label != NULL)
			? conf->button.ok_label : "OK";
#ifdef WITH_XDIALOG
		if(opt != NULL && opt->wizard)
			label = "Next";
#endif
		gtk_dialog_add_button(GTK_DIALOG(dialog), label,
				GTK_RESPONSE_OK);
	}
	if(conf->button.with_help == true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf->button.help_label != NULL)
				? conf->button.help_label : "Help",
				GTK_RESPONSE_HELP);
#ifdef WITH_XDIALOG
	if(opt != NULL && opt->wizard)
		gtk_dialog_add_button(GTK_DIALOG(dialog), "Previous", 3);
#endif
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
			(conf->button.without_cancel != true
			 && conf->button.default_cancel)
			? GTK_RESPONSE_CANCEL : GTK_RESPONSE_OK);
}


/* builder_dialog_error */
static int _builder_dialog_error(GtkWidget * parent,
		struct bsddialog_conf const * conf, struct options const * opt,
		char const * error)
{
	GtkWidget * dialog;
#if GTK_CHECK_VERSION(3, 12, 0)
	const GtkDialogFlags flags = GTK_DIALOG_USE_HEADER_BAR;
#else
	const GtkDialogFlags flags = 0;
#endif

	dialog = gtk_message_dialog_new(parent ? GTK_WINDOW(parent) : NULL,
			flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", "Error");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", error);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	if(parent == NULL && conf != NULL)
	{
		if(conf->x == BSDDIALOG_FULLSCREEN
				|| conf->y == BSDDIALOG_FULLSCREEN)
			gtk_window_fullscreen(GTK_WINDOW(dialog));
		else if(conf->x > 0 && conf->y > 0)
			gtk_window_move(GTK_WINDOW(dialog), conf->x, conf->y);
		else if(opt != NULL)
			gtk_window_set_position(GTK_WINDOW(dialog),
					opt->position);
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
		struct bsddialog_conf const * conf,
		struct options const * opt)
{
	int ret;
	struct bsddialog_conf conf2;
	GtkWidget * dialog;
	const GtkDialogFlags flags = 0;
	GtkButtonsType buttons = GTK_BUTTONS_CLOSE;
#ifndef WITH_XDIALOG
	(void) opt;
#endif

	if(conf->key.f1_file != NULL)
	{
		memset(&conf2, 0, sizeof(conf2));
		conf2.title = "Help";
		return builder_textbox(&conf2, conf->key.f1_file, 0, 0, 0, NULL,
				NULL);
	}
#ifdef WITH_XDIALOG
	if(parent != NULL && opt->help != NULL && opt->help[0] == '\0')
	{
		gtk_dialog_response(GTK_DIALOG(parent), GTK_RESPONSE_HELP);
		return 0;
	}
#endif
	dialog = gtk_message_dialog_new((parent != NULL)
			? GTK_WINDOW(parent) : NULL, flags,
			GTK_MESSAGE_QUESTION, buttons,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", "Help");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", conf->key.f1_message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Help");
	ret = _builder_dialog_run(conf, dialog);
	gtk_widget_destroy(dialog);
	return ret;
}


/* builder_dialog_menu_output */
static int _builder_dialog_menu_output(struct options const * opt,
		GtkTreeSelection * treesel, GtkTreeModel * model,
		unsigned int id, char const * prefix)
{
	GtkTreeIter iter;
	gchar * p;
	gboolean toquote;
	char quotech;

	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) == FALSE)
		return BSDDIALOG_HELP;
	gtk_tree_model_get(model, &iter, id, &p, -1);
	if(prefix != NULL)
		dprintf(opt->output_fd, "%s", prefix);
	toquote = string_needs_quoting(p) ? opt->item_always_quote : FALSE;
	if(toquote)
	{
		quotech = opt->item_singlequote ? '\'' : '"';
		dprintf(opt->output_fd, "%c%s%c\n", quotech, p, quotech);
	}
	else
		dprintf(opt->output_fd, "%s\n", p);
	free(p);
	return BSDDIALOG_HELP;
}


/* builder_dialog_run */
static int _builder_dialog_run(struct bsddialog_conf const * conf,
		GtkWidget * dialog)
{
	int res;

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if(conf->get_height != NULL || conf->get_width != NULL)
		gtk_window_get_size(GTK_WINDOW(dialog),
				conf->get_width, conf->get_height);
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
