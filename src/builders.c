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
#include <gtk/gtk.h>
#include "callbacks.h"
#include "builders.h"


/* builders */
/* types */
struct infobox_data
{
	GtkWidget * dialog;
	unsigned int id;
};

struct pause_data
{
	GtkWidget * dialog;
	GtkWidget * widget;
	unsigned int secs;
	gdouble step;
	unsigned int id;
};


/* prototypes */
static GtkWidget * _builder_dialog(struct bsddialog_conf const * conf,
		char const * text);
static void _builder_dialog_buttons(GtkWidget * dialog,
		struct bsddialog_conf const * conf);
static int _builder_dialog_output(struct bsddialog_conf const * conf,
		struct options const * opt, int res);
static int _builder_dialog_run(GtkWidget * dialog);


/* functions */
/* builder_checklist */
static void _checklist_on_row_activated(gpointer data);

int builder_checklist(struct bsddialog_conf const * conf,
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
	int i, n, res;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %d, %d (%d), \"%s\")\n", __func__, rows, cols,
			argc, (argc - 1) / 3,
			(argv[0] != NULL) ? argv[0] : "(null)");
#endif
	if(argc < 1)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	if((n = strtol(argv[0], NULL, 10)) > (argc - 1) / 3)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	else if(n == 0)
		n = (argc - 1) / 3;
	dialog = _builder_dialog(conf, text);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	store = gtk_list_store_new(3, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_STRING);
	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	widget = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(widget), GTK_TREE_MODEL(store));
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_SINGLE);
	for(i = 0; i < n; i++)
	{
		gtk_list_store_insert(store, &iter, -1);
		gtk_list_store_set(store, &iter,
				0, strcasecmp(argv[i * 3 + 3], "on") == 0,
				1, argv[i * 3 + 1], 2, argv[i * 3 + 2], -1);
		if(opt->item_default != NULL
				&& strcmp(argv[i * 3 + 1], opt->item_default) == 0)
			gtk_tree_selection_select_iter(treesel, &iter);
	}
	column = gtk_tree_view_column_new_with_attributes(NULL,
			gtk_cell_renderer_toggle_new(), "active", 0, NULL);
	gtk_tree_view_column_set_expand(column, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	column = gtk_tree_view_column_new_with_attributes(NULL,
			gtk_cell_renderer_text_new(), "text", 1, NULL);
	gtk_tree_view_column_set_expand(column, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	column = gtk_tree_view_column_new_with_attributes(NULL,
			gtk_cell_renderer_text_new(), "text", 2, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	g_signal_connect_swapped(widget, "row-activated",
			G_CALLBACK(_checklist_on_row_activated), dialog);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_container_add(GTK_CONTAINER(container), window);
	gtk_widget_show_all(window);
	_builder_dialog_buttons(dialog, conf);
	res = _builder_dialog_run(dialog);
	/* FIXME implement the output */
	gtk_widget_destroy(dialog);
	return _builder_dialog_output(conf, opt, res);
}

static void _checklist_on_row_activated(gpointer data)
{
	GtkWidget * dialog = data;

	/* FIXME may be a different button */
	gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
}


/* builder_gauge */
int builder_gauge(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * widget;
	GtkWidget * dialog;
	GtkWidget * container;
	gdouble fraction = 0.0;
	char buf[8];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, argc);
#endif
	if(argc > 1)
		error_args(opt->name, argc - 1, argv + 1);
	else if(argc == 1)
		fraction = (gdouble)(strtoul(argv[0], NULL, 10)) / 100.0;
	dialog = _builder_dialog(conf, text);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	widget = gtk_progress_bar_new();
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(widget), TRUE);
	snprintf(buf, sizeof(buf), "%.0lf %%", fraction * 100.0);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), buf);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget), fraction);
	gtk_widget_show(widget);
	gtk_container_add(GTK_CONTAINER(container), widget);
	/* FIXME read input */
	ret = _builder_dialog_run(dialog);
	gtk_widget_destroy(dialog);
	return ret;
}


/* builder_infobox */
static gboolean _infobox_on_timeout(gpointer data);

int builder_infobox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	GtkButtonsType buttons = GTK_BUTTONS_OK;
	struct infobox_data id = { NULL, 0 };

	if(argc > 0)
		error_args(opt->name, argc, argv);
	if(conf->sleep > 0)
	{
		buttons = GTK_BUTTONS_NONE;
		id.id = g_timeout_add(conf->sleep * 1000,
				G_CALLBACK(_infobox_on_timeout), &id);
	}
	id.dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_INFO,
			buttons, "%s", text);
	if(conf->title != NULL)
		gtk_window_set_title(GTK_WINDOW(id.dialog), conf->title);
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
static void _inputbox_on_activate(GtkWidget * widget);
static void _inputbox_on_activate_cancel(GtkWidget * widget);

int builder_inputbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * widget;
	GtkEntryBuffer * buffer;

	if(argc > 0)
		error_args(opt->name, argc, argv);
	dialog = _builder_dialog(conf, text);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	buffer = gtk_entry_buffer_new(NULL, -1);
	widget = gtk_entry_new_with_buffer(buffer);
	/* FIXME may be a different button (or never cancel) */
	g_signal_connect_swapped(widget, "activate", conf->button.default_cancel
			? G_CALLBACK(_inputbox_on_activate_cancel)
			: G_CALLBACK(_inputbox_on_activate), dialog);
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

static void _inputbox_on_activate(GtkWidget * widget)
{
	gtk_dialog_response(GTK_DIALOG(widget), GTK_RESPONSE_OK);
}

static void _inputbox_on_activate_cancel(GtkWidget * widget)
{
	gtk_dialog_response(GTK_DIALOG(widget), GTK_RESPONSE_CANCEL);
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
	int i, n, res;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %d, %d (%d), \"%s\")\n", __func__, rows, cols,
			argc, (argc - 1) / 2,
			(argv[0] != NULL) ? argv[0] : "(null)");
#endif
	if(argc < 1)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	if((n = strtol(argv[0], NULL, 10)) > (argc - 1) / 2)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	else if(n == 0)
		n = (argc - 1) / 2;
	dialog = _builder_dialog(conf, text);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	widget = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(widget), GTK_TREE_MODEL(store));
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_SINGLE);
	for(i = 0; i < n; i++)
	{
		gtk_list_store_insert(store, &iter, -1);
		gtk_list_store_set(store, &iter,
				0, argv[i * 2 + 1], 1, argv[i * 2 + 2], -1);
		if(opt->item_default != NULL
				&& strcmp(argv[i * 2 + 1], opt->item_default) == 0)
			gtk_tree_selection_select_iter(treesel, &iter);
	}
	column = gtk_tree_view_column_new_with_attributes(NULL,
			gtk_cell_renderer_text_new(), "text", 0, NULL);
	gtk_tree_view_column_set_expand(column, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	column = gtk_tree_view_column_new_with_attributes(NULL,
			gtk_cell_renderer_text_new(), "text", 1, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	g_signal_connect_swapped(widget, "row-activated",
			G_CALLBACK(_menu_on_row_activated), dialog);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_container_add(GTK_CONTAINER(container), window);
	gtk_widget_show_all(window);
	_builder_dialog_buttons(dialog, conf);
	res = _builder_dialog_run(dialog);
	if((res == BSDDIALOG_OK || res == BSDDIALOG_EXTRA)
			&& gtk_tree_selection_get_selected(treesel, NULL,
				&iter) == TRUE)
	{
		gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 0, &p, -1);
		dprintf(opt->output_fd, "%s\n", p);
		free(p);
		gtk_widget_destroy(dialog);
		return res;
	}
	gtk_widget_destroy(dialog);
	return _builder_dialog_output(conf, opt, res);
}

static void _menu_on_row_activated(gpointer data)
{
	GtkWidget * dialog = data;

	/* FIXME may be a different button */
	gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
}


/* builder_msgbox */
int builder_msgbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	GtkWidget * dialog;
	int res;

	if(argc > 0)
		error_args(opt->name, argc, argv);
	dialog = _builder_dialog(conf, text);
	_builder_dialog_buttons(dialog, conf);
	res = _builder_dialog_run(dialog);
	gtk_widget_destroy(dialog);
	return _builder_dialog_output(conf, opt, res);
}


/* builder_passwordbox */
static void _passwordbox_on_activate(GtkWidget * widget);
static void _passwordbox_on_activate_cancel(GtkWidget * widget);

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
		error_args(opt->name, argc, argv);
	dialog = _builder_dialog(conf, text);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	buffer = gtk_entry_buffer_new(NULL, -1);
	widget = gtk_entry_new_with_buffer(buffer);
	gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
	/* FIXME may be a different button (or never cancel) */
	g_signal_connect_swapped(widget, "activate", conf->button.default_cancel
			? G_CALLBACK(_passwordbox_on_activate_cancel)
			: G_CALLBACK(_passwordbox_on_activate), dialog);
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

static void _passwordbox_on_activate(GtkWidget * widget)
{
	gtk_dialog_response(GTK_DIALOG(widget), GTK_RESPONSE_OK);
}

static void _passwordbox_on_activate_cancel(GtkWidget * widget)
{
	gtk_dialog_response(GTK_DIALOG(widget), GTK_RESPONSE_CANCEL);
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
		error_args(opt->name, argc, argv);
	if(argc > 1)
		error_args(opt->name, argc - 1, argv + 1);
	pd.secs = strtoul(argv[0], NULL, 10);
	pd.dialog = _builder_dialog(conf, text);
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


/* builder_yesno */
int builder_yesno(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	GtkWidget * dialog;
	int ret;

	if(argc > 0)
		error_args(opt->name, argc, argv);
	dialog = _builder_dialog(conf, text);
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
		char const * text)
{
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * widget;

	dialog = gtk_dialog_new();
	if(conf->title != NULL)
		gtk_window_set_title(GTK_WINDOW(dialog), conf->title);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	if(text != NULL)
	{
		widget = gtk_label_new(text);
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
			dprintf(opt->output_fd, "%s\n", conf->button.cancel_label
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
