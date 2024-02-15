/* Xdialog */
/* builders.c */
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



#ifndef WITH_XDIALOG
# define WITH_XDIALOG
#endif

#include "../src/callbacks.c"
#include "../src/builders.c"


/* private */
/* types */
struct buildlist_data
{
	/* left treeview */
	GtkListStore * lstore;
	GtkTreeSelection * ltreesel;

	/* right treeview */
	GtkListStore * rstore;
	GtkTreeSelection * rtreesel;
};

struct progress_data
{
	struct options const * opt;
	GtkWidget * dialog;
	GtkWidget * label;
	GtkWidget * widget;	/* progress bar */
	guint id;
	unsigned int count;
	unsigned int maxdots;
	int msglen;
};

struct logbox_data
{
	struct options const * opt;

	char const * filename;
	int fd;
	GtkWidget * dialog;
	GtkListStore * store;
	GtkWidget * view;
	guint id;
	GIOChannel * channel;
};


/* constants */
enum BUILDLIST_LIST_STORE
{
	BLS_ITEM = 0,
	BLS_NAME,
	BLS_TOOLTIP
};
# define BLS_LAST BLS_TOOLTIP
# define BLS_COUNT (BLS_LAST + 1)


/* prototypes */
static int _builder_dialog_fselect(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt,
		GtkFileChooserAction action);


/* public */
/* functions */
/* builder_2inputsbox */
int builder_2inputsbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * box;
	GtkWidget * widget;
	GtkEntryBuffer * buffer1;
	GtkEntryBuffer * buffer2;
	GtkSizeGroup * group;
	char const * sep = (opt->item_output_sep != NULL)
		? opt->item_output_sep : "/";

	if(argc != 4)
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
	/* input 1 */
#if GTK_CHECK_VERSION(3, 0, 0)
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, BORDER_WIDTH);
#else
	box = gtk_hbox_new(FALSE, BORDER_WIDTH);
#endif
	widget = gtk_label_new(argv[0]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
	buffer1 = gtk_entry_buffer_new(argv[1], -1);
	widget = gtk_entry_new_with_buffer(buffer1);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	if(opt->max_input_form > 0)
		gtk_entry_set_max_length(GTK_ENTRY(widget),
				opt->max_input_form);
	if(opt->password & (0x1 << 0))
		gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
	if(cols > 0)
		gtk_entry_set_width_chars(GTK_ENTRY(widget), cols);
	gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(container), box);
	/* input 2 */
#if GTK_CHECK_VERSION(3, 0, 0)
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, BORDER_WIDTH);
#else
	box = gtk_hbox_new(FALSE, BORDER_WIDTH);
#endif
	widget = gtk_label_new(argv[2]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
	buffer2 = gtk_entry_buffer_new(argv[3], -1);
	widget = gtk_entry_new_with_buffer(buffer2);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	if(opt->max_input_form > 0)
		gtk_entry_set_max_length(GTK_ENTRY(widget),
				opt->max_input_form);
	if(opt->password & (0x1 << 1))
		gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
	if(cols > 0)
		gtk_entry_set_width_chars(GTK_ENTRY(widget), cols);
	gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(container), box);
	gtk_widget_show_all(container);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			dprintf(opt->output_fd, "%s%s%s\n",
					gtk_entry_buffer_get_text(buffer1), sep,
					gtk_entry_buffer_get_text(buffer2));
			break;
	}
	g_object_unref(buffer1);
	g_object_unref(buffer2);
	return ret;
}


/* builder_2rangesbox */
int builder_2rangesbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * box;
	GtkWidget * widget1, * widget2;
	int min, max, value1, value2;
	char const * sep = (opt->item_output_sep != NULL)
		? opt->item_output_sep : "/";

	if(argc != 8)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	box = dialog->vbox;
#endif
	/* range 1 */
	widget1 = gtk_label_new(argv[0]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget1, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget1), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget1), opt->justify);
	gtk_box_pack_start(GTK_BOX(box), widget1, FALSE, TRUE, BORDER_WIDTH);
	min = strtol(argv[1], NULL, 10);
	max = strtol(argv[2], NULL, 10);
	if(min > max)
	{
		printf("Error: min1 > max1\n");
		exit(EXITCODE(BSDDIALOG_ERROR));
		return BSDDIALOG_ERROR;
	}
	value1 = strtol(argv[3], NULL, 10);
#if GTK_CHECK_VERSION(3, 0, 0)
	widget1 = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
			(gdouble)min, (gdouble)max, 1.0);
#else
	widget1 = gtk_hscale_new_with_range((gdouble)min, (gdouble)max, 1.0);
#endif
	gtk_range_set_value(GTK_RANGE(widget1), (gdouble)value1);
	gtk_box_pack_start(GTK_BOX(box), widget1, FALSE, TRUE, BORDER_WIDTH);
	/* range 2 */
	widget2 = gtk_label_new(argv[4]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget2, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget2), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget2), opt->justify);
	gtk_box_pack_start(GTK_BOX(box), widget2, FALSE, TRUE, BORDER_WIDTH);
	min = strtol(argv[5], NULL, 10);
	max = strtol(argv[6], NULL, 10);
	if(min > max)
	{
		printf("Error: min2 > max2\n");
		exit(EXITCODE(BSDDIALOG_ERROR));
		return BSDDIALOG_ERROR;
	}
	value2 = strtol(argv[7], NULL, 10);
#if GTK_CHECK_VERSION(3, 0, 0)
	widget2 = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
			(gdouble)min, (gdouble)max, 1.0);
#else
	widget2 = gtk_hscale_new_with_range((gdouble)min, (gdouble)max, 1.0);
#endif
	gtk_range_set_value(GTK_RANGE(widget2), (gdouble)value2);
	gtk_box_pack_start(GTK_BOX(box), widget2, FALSE, TRUE, BORDER_WIDTH);
	_builder_dialog_buttons(dialog, conf, opt);
	gtk_widget_show_all(box);
	ret = _builder_dialog_run(conf, dialog);
	value1 = gtk_range_get_value(GTK_RANGE(widget1));
	value2 = gtk_range_get_value(GTK_RANGE(widget2));
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			dprintf(opt->output_fd, "%d%s%d\n",
					value1, sep, value2);
			break;
	}
	return ret;
}


/* builder_2spinsbox */
int builder_2spinsbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * box;
	GtkWidget * widget, * widget1, * widget2;
	int min, max, value1, value2;
	char const * sep = (opt->item_output_sep != NULL)
		? opt->item_output_sep : "/";

	if(argc != 8)
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
#if GTK_CHECK_VERSION(3, 0, 0)
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, BORDER_WIDTH);
#else
	box = gtk_hbox_new(FALSE, BORDER_WIDTH);
#endif
	/* spin 1 */
	min = strtol(argv[0], NULL, 10);
	max = strtol(argv[1], NULL, 10);
	if(min > max)
	{
		printf("Error: min1 > max1\n");
		exit(EXITCODE(BSDDIALOG_ERROR));
		return BSDDIALOG_ERROR;
	}
	value1 = strtol(argv[2], NULL, 10);
	widget1 = gtk_spin_button_new_with_range((gdouble)min, (gdouble)max,
			1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget1), (gdouble)value1);
	gtk_entry_set_activates_default(GTK_ENTRY(widget1), TRUE);
	gtk_box_pack_start(GTK_BOX(box), widget1, FALSE, TRUE, BORDER_WIDTH);
	widget = gtk_label_new(argv[3]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, BORDER_WIDTH);
	/* spin 2 */
	min = strtol(argv[4], NULL, 10);
	max = strtol(argv[5], NULL, 10);
	if(min > max)
	{
		printf("Error: min2 > max2\n");
		exit(EXITCODE(BSDDIALOG_ERROR));
		return BSDDIALOG_ERROR;
	}
	value2 = strtol(argv[6], NULL, 10);
	widget2 = gtk_spin_button_new_with_range((gdouble)min, (gdouble)max,
			1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget2), (gdouble)value2);
	gtk_entry_set_activates_default(GTK_ENTRY(widget2), TRUE);
	gtk_box_pack_start(GTK_BOX(box), widget2, FALSE, TRUE, BORDER_WIDTH);
	widget = gtk_label_new(argv[7]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, BORDER_WIDTH);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, TRUE, BORDER_WIDTH);
	_builder_dialog_buttons(dialog, conf, opt);
	gtk_widget_show_all(box);
	ret = _builder_dialog_run(conf, dialog);
	value1 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget1));
	value2 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget2));
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			dprintf(opt->output_fd, "%d%s%d\n",
					value1, sep, value2);
			break;
	}
	return ret;
}


/* builder_3inputsbox */
int builder_3inputsbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * box;
	GtkWidget * widget;
	GtkEntryBuffer * buffer1;
	GtkEntryBuffer * buffer2;
	GtkEntryBuffer * buffer3;
	GtkSizeGroup * group;
	char const * sep = (opt->item_output_sep != NULL)
		? opt->item_output_sep : "/";

	if(argc != 6)
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
	/* input 1 */
#if GTK_CHECK_VERSION(3, 0, 0)
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, BORDER_WIDTH);
#else
	box = gtk_hbox_new(FALSE, BORDER_WIDTH);
#endif
	widget = gtk_label_new(argv[0]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
	buffer1 = gtk_entry_buffer_new(argv[1], -1);
	widget = gtk_entry_new_with_buffer(buffer1);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	if(opt->max_input_form > 0)
		gtk_entry_set_max_length(GTK_ENTRY(widget),
				opt->max_input_form);
	if(opt->password & (0x1 << 0))
		gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
	if(cols > 0)
		gtk_entry_set_width_chars(GTK_ENTRY(widget), cols);
	gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(container), box);
	/* input 2 */
#if GTK_CHECK_VERSION(3, 0, 0)
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, BORDER_WIDTH);
#else
	box = gtk_hbox_new(FALSE, BORDER_WIDTH);
#endif
	widget = gtk_label_new(argv[2]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
	buffer2 = gtk_entry_buffer_new(argv[3], -1);
	widget = gtk_entry_new_with_buffer(buffer2);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	if(opt->max_input_form > 0)
		gtk_entry_set_max_length(GTK_ENTRY(widget),
				opt->max_input_form);
	if(opt->password & (0x1 << 1))
		gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
	if(cols > 0)
		gtk_entry_set_width_chars(GTK_ENTRY(widget), cols);
	gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(container), box);
	/* input 3 */
#if GTK_CHECK_VERSION(3, 0, 0)
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, BORDER_WIDTH);
#else
	box = gtk_hbox_new(FALSE, BORDER_WIDTH);
#endif
	widget = gtk_label_new(argv[4]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
	buffer3 = gtk_entry_buffer_new(argv[5], -1);
	widget = gtk_entry_new_with_buffer(buffer3);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	if(opt->max_input_form > 0)
		gtk_entry_set_max_length(GTK_ENTRY(widget),
				opt->max_input_form);
	if(opt->password & (0x1 << 2))
		gtk_entry_set_visibility(GTK_ENTRY(widget), FALSE);
	if(cols > 0)
		gtk_entry_set_width_chars(GTK_ENTRY(widget), cols);
	gtk_box_pack_start(GTK_BOX(box), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(container), box);
	gtk_widget_show_all(container);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			dprintf(opt->output_fd, "%s%s%s%s%s\n",
					gtk_entry_buffer_get_text(buffer1), sep,
					gtk_entry_buffer_get_text(buffer2), sep,
					gtk_entry_buffer_get_text(buffer3));
			break;
	}
	g_object_unref(buffer1);
	g_object_unref(buffer2);
	g_object_unref(buffer3);
	return ret;
}


/* builder_3rangesbox */
int builder_3rangesbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * box;
	GtkWidget * widget1, * widget2, * widget3;
	int min, max, value1, value2, value3;
	char const * sep = (opt->item_output_sep != NULL)
		? opt->item_output_sep : "/";

	if(argc != 12)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, opt, text, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	box = dialog->vbox;
#endif
	/* range 1 */
	widget1 = gtk_label_new(argv[0]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget1, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget1), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget1), opt->justify);
	gtk_box_pack_start(GTK_BOX(box), widget1, FALSE, TRUE, BORDER_WIDTH);
	min = strtol(argv[1], NULL, 10);
	max = strtol(argv[2], NULL, 10);
	if(min > max)
	{
		printf("Error: min1 > max1\n");
		exit(EXITCODE(BSDDIALOG_ERROR));
		return BSDDIALOG_ERROR;
	}
	value1 = strtol(argv[3], NULL, 10);
#if GTK_CHECK_VERSION(3, 0, 0)
	widget1 = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
			(gdouble)min, (gdouble)max, 1.0);
#else
	widget1 = gtk_hscale_new_with_range((gdouble)min, (gdouble)max, 1.0);
#endif
	gtk_range_set_value(GTK_RANGE(widget1), (gdouble)value1);
	gtk_box_pack_start(GTK_BOX(box), widget1, FALSE, TRUE, BORDER_WIDTH);
	/* range 2 */
	widget2 = gtk_label_new(argv[4]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget2, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget2), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget2), opt->justify);
	gtk_box_pack_start(GTK_BOX(box), widget2, FALSE, TRUE, BORDER_WIDTH);
	min = strtol(argv[5], NULL, 10);
	max = strtol(argv[6], NULL, 10);
	if(min > max)
	{
		printf("Error: min2 > max2\n");
		exit(EXITCODE(BSDDIALOG_ERROR));
		return BSDDIALOG_ERROR;
	}
	value2 = strtol(argv[7], NULL, 10);
#if GTK_CHECK_VERSION(3, 0, 0)
	widget2 = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
			(gdouble)min, (gdouble)max, 1.0);
#else
	widget2 = gtk_hscale_new_with_range((gdouble)min, (gdouble)max, 1.0);
#endif
	gtk_range_set_value(GTK_RANGE(widget2), (gdouble)value2);
	gtk_box_pack_start(GTK_BOX(box), widget2, FALSE, TRUE, BORDER_WIDTH);
	/* range 3 */
	widget3 = gtk_label_new(argv[8]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget3, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget3), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget3), opt->justify);
	gtk_box_pack_start(GTK_BOX(box), widget3, FALSE, TRUE, BORDER_WIDTH);
	min = strtol(argv[9], NULL, 10);
	max = strtol(argv[10], NULL, 10);
	if(min > max)
	{
		printf("Error: min3 > max3\n");
		exit(EXITCODE(BSDDIALOG_ERROR));
		return BSDDIALOG_ERROR;
	}
	value3 = strtol(argv[11], NULL, 10);
#if GTK_CHECK_VERSION(3, 0, 0)
	widget3 = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
			(gdouble)min, (gdouble)max, 1.0);
#else
	widget3 = gtk_hscale_new_with_range((gdouble)min, (gdouble)max, 1.0);
#endif
	gtk_range_set_value(GTK_RANGE(widget3), (gdouble)value3);
	gtk_box_pack_start(GTK_BOX(box), widget3, FALSE, TRUE, BORDER_WIDTH);
	_builder_dialog_buttons(dialog, conf, opt);
	gtk_widget_show_all(box);
	ret = _builder_dialog_run(conf, dialog);
	value1 = gtk_range_get_value(GTK_RANGE(widget1));
	value2 = gtk_range_get_value(GTK_RANGE(widget2));
	value3 = gtk_range_get_value(GTK_RANGE(widget3));
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			dprintf(opt->output_fd, "%d%s%d%s%d\n",
					value1, sep, value2, sep, value3);
			break;
	}
	return ret;
}


/* builder_3spinsbox */
int builder_3spinsbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * box;
	GtkWidget * widget, * widget1, * widget2, * widget3;
	int min, max, value1, value2, value3;
	char const * sep = (opt->item_output_sep != NULL)
		? opt->item_output_sep : "/";

	if(argc != 12)
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
#if GTK_CHECK_VERSION(3, 0, 0)
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, BORDER_WIDTH);
#else
	box = gtk_hbox_new(FALSE, BORDER_WIDTH);
#endif
	/* spin 1 */
	min = strtol(argv[0], NULL, 10);
	max = strtol(argv[1], NULL, 10);
	if(min > max)
	{
		printf("Error: min1 > max1\n");
		exit(EXITCODE(BSDDIALOG_ERROR));
		return BSDDIALOG_ERROR;
	}
	value1 = strtol(argv[2], NULL, 10);
	widget1 = gtk_spin_button_new_with_range((gdouble)min, (gdouble)max,
			1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget1), (gdouble)value1);
	gtk_entry_set_activates_default(GTK_ENTRY(widget1), TRUE);
	gtk_box_pack_start(GTK_BOX(box), widget1, FALSE, TRUE, BORDER_WIDTH);
	widget = gtk_label_new(argv[3]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, BORDER_WIDTH);
	/* spin 2 */
	min = strtol(argv[4], NULL, 10);
	max = strtol(argv[5], NULL, 10);
	if(min > max)
	{
		printf("Error: min2 > max2\n");
		exit(EXITCODE(BSDDIALOG_ERROR));
		return BSDDIALOG_ERROR;
	}
	value2 = strtol(argv[6], NULL, 10);
	widget2 = gtk_spin_button_new_with_range((gdouble)min, (gdouble)max,
			1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget2), (gdouble)value2);
	gtk_entry_set_activates_default(GTK_ENTRY(widget2), TRUE);
	gtk_box_pack_start(GTK_BOX(box), widget2, FALSE, TRUE, BORDER_WIDTH);
	widget = gtk_label_new(argv[7]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, BORDER_WIDTH);
	/* spin 3 */
	min = strtol(argv[8], NULL, 10);
	max = strtol(argv[9], NULL, 10);
	if(min > max)
	{
		printf("Error: min3 > max3\n");
		exit(EXITCODE(BSDDIALOG_ERROR));
		return BSDDIALOG_ERROR;
	}
	value3 = strtol(argv[10], NULL, 10);
	widget3 = gtk_spin_button_new_with_range((gdouble)min, (gdouble)max,
			1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget3), (gdouble)value3);
	gtk_entry_set_activates_default(GTK_ENTRY(widget3), TRUE);
	gtk_box_pack_start(GTK_BOX(box), widget3, FALSE, TRUE, BORDER_WIDTH);
	widget = gtk_label_new(argv[11]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget, opt->halign);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), opt->halign, 0.5);
#endif
	gtk_label_set_justify(GTK_LABEL(widget), opt->justify);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, BORDER_WIDTH);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, TRUE, BORDER_WIDTH);
	_builder_dialog_buttons(dialog, conf, opt);
	gtk_widget_show_all(box);
	ret = _builder_dialog_run(conf, dialog);
	value1 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget1));
	value2 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget2));
	value3 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget3));
	gtk_widget_destroy(dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			dprintf(opt->output_fd, "%d%s%d%s%d\n",
					value1, sep, value2, sep, value3);
			break;
	}
	return ret;
}


/* builder_buildlist */
static void _buildlist_convert_rows(GtkTreeModel * model, GList * rows);
static void _buildlist_on_add(gpointer data);
static void _buildlist_on_remove(gpointer data);

int builder_buildlist(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	struct buildlist_data bd;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * hbox;
	GtkWidget * window;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkTreeViewColumn * column;
	GtkListStore * store;
	GtkTreeIter iter;
	gboolean valid;
	gchar * p;
	int i, j = 3, k, n;
	char const * sep = "";

	if(opt->item_bottomdesc == true)
		j++;
	if(argc < (j + 1) || (n = strtol(argv[0], NULL, 10)) < 0
			|| ((argc - 1) % j) != 0)
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
#if GTK_CHECK_VERSION(3, 0, 0)
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, BORDER_WIDTH);
#else
	hbox = gtk_hbox_new(FALSE, BORDER_WIDTH);
#endif
	/* left treeview */
	window = gtk_scrolled_window_new(NULL, NULL);
	bd.lstore = gtk_list_store_new(BLS_COUNT, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING);
	widget = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(widget),
			GTK_TREE_MODEL(bd.lstore));
	if(opt->item_bottomdesc)
		gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(widget),
				BLS_TOOLTIP);
	bd.ltreesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(bd.ltreesel, GTK_SELECTION_MULTIPLE);
	column = gtk_tree_view_column_new_with_attributes(NULL,
			gtk_cell_renderer_text_new(), "text", BLS_NAME, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_box_pack_start(GTK_BOX(hbox), window, TRUE, TRUE, 0);
	/* middle buttons */
#if GTK_CHECK_VERSION(3, 0, 0)
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, BORDER_WIDTH);
#else
	vbox = gtk_vbox_new(FALSE, BORDER_WIDTH);
#endif
	widget = gtk_button_new_with_label("Add");
	g_signal_connect_swapped(widget, "clicked",
			G_CALLBACK(_buildlist_on_add), &bd);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	widget = gtk_button_new_with_label("Remove");
	g_signal_connect_swapped(widget, "clicked",
			G_CALLBACK(_buildlist_on_remove), &bd);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, TRUE, 0);
	/* right treeview */
	window = gtk_scrolled_window_new(NULL, NULL);
	bd.rstore = gtk_list_store_new(BLS_COUNT, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING);
	widget = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget), FALSE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(widget),
			GTK_TREE_MODEL(bd.rstore));
	if(opt->item_bottomdesc)
		gtk_tree_view_set_tooltip_column(GTK_TREE_VIEW(widget),
				BLS_TOOLTIP);
	bd.rtreesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_set_mode(bd.rtreesel, GTK_SELECTION_MULTIPLE);
	column = gtk_tree_view_column_new_with_attributes(NULL,
			gtk_cell_renderer_text_new(), "text", BLS_NAME, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);
	gtk_container_add(GTK_CONTAINER(window), widget);
	gtk_box_pack_start(GTK_BOX(hbox), window, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(container), hbox, TRUE, TRUE, 0);
	/* fill the data */
	for(i = 0; (i + 1) * j < argc; i++)
	{
		k = i * j + 1;
		store = (strcmp(argv[k + 2], "on") == 0)
			? bd.rstore : bd.lstore;
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, BLS_ITEM, argv[k],
				BLS_NAME, argv[k + 1], -1);
		if(opt->item_bottomdesc == true)
			gtk_list_store_set(store, &iter,
					BLS_TOOLTIP, argv[k + 3], -1);
	}
	gtk_widget_show_all(container);
	_builder_dialog_buttons(dialog, conf, NULL);
	ret = _builder_dialog_run(conf, dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			for(valid = gtk_tree_model_get_iter_first(
						GTK_TREE_MODEL(bd.rstore),
						&iter); valid == TRUE;
					valid = gtk_tree_model_iter_next(
						GTK_TREE_MODEL(bd.rstore),
						&iter))
			{
				gtk_tree_model_get(GTK_TREE_MODEL(bd.rstore),
						&iter, BLS_ITEM, &p, -1);
				dprintf(opt->output_fd, "%s%s", sep, p);
				sep = (opt->item_output_sep != NULL)
					? opt->item_output_sep : "/";
				g_free(p);
			}
			if(strlen(sep) > 0)
				dprintf(opt->output_fd, "\n");
			break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}

static void _buildlist_convert_rows(GtkTreeModel * model, GList * rows)
{
	GList * row;
	GtkTreeRowReference * ref;

	for(row = rows; row != NULL; row = row->next)
	{
		ref = gtk_tree_row_reference_new(model, row->data);
		gtk_tree_path_free(row->data);
		row->data = ref;
	}
}

static void _buildlist_on_add(gpointer data)
{
	struct buildlist_data * bd = data;
	GList * rows, * row;
	GtkTreePath * path;
	GtkTreeIter iter;
	gchar * item;
	gchar * name;
	gchar * help;

	rows = gtk_tree_selection_get_selected_rows(bd->ltreesel, NULL);
	_buildlist_convert_rows(GTK_TREE_MODEL(bd->lstore), rows);
	for(row = rows; row != NULL; row = row->next)
	{
		if((path = gtk_tree_row_reference_get_path(row->data)) == NULL)
			continue;
		gtk_tree_model_get_iter(GTK_TREE_MODEL(bd->lstore), &iter,
				path);
		gtk_tree_path_free(path);
		gtk_tree_model_get(GTK_TREE_MODEL(bd->lstore), &iter,
				BLS_ITEM, &item, BLS_NAME, &name,
				BLS_TOOLTIP, &help, -1);
		gtk_list_store_remove(bd->lstore, &iter);
		gtk_list_store_append(bd->rstore, &iter);
		gtk_list_store_set(bd->rstore, &iter, BLS_ITEM, item,
				BLS_NAME, name, BLS_TOOLTIP, help, -1);
		g_free(item);
		g_free(name);
		g_free(help);
	}
	g_list_foreach(rows, (GFunc)gtk_tree_row_reference_free, NULL);
	g_list_free(rows);
}

static void _buildlist_on_remove(gpointer data)
{
	struct buildlist_data * bd = data;
	GList * rows, * row;
	GtkTreePath * path;
	GtkTreeIter iter;
	gchar * item;
	gchar * name;
	gchar * help;

	rows = gtk_tree_selection_get_selected_rows(bd->rtreesel, NULL);
	_buildlist_convert_rows(GTK_TREE_MODEL(bd->rstore), rows);
	for(row = rows; row != NULL; row = row->next)
	{
		path = gtk_tree_row_reference_get_path(row->data);
		gtk_tree_model_get_iter(GTK_TREE_MODEL(bd->rstore), &iter,
				path);
		gtk_tree_path_free(path);
		gtk_tree_model_get(GTK_TREE_MODEL(bd->rstore), &iter,
				BLS_ITEM, &item, BLS_NAME, &name,
				BLS_TOOLTIP, &help, -1);
		gtk_list_store_remove(bd->rstore, &iter);
		gtk_list_store_append(bd->lstore, &iter);
		gtk_list_store_set(bd->lstore, &iter, BLS_ITEM, item,
				BLS_NAME, name, BLS_TOOLTIP, help, -1);
		g_free(item);
		g_free(name);
		g_free(help);
	}
	g_list_foreach(rows, (GFunc)gtk_tree_row_reference_free, NULL);
	g_list_free(rows);
}


/* colorsel */
int builder_colorsel(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * widget;
#if GTK_CHECK_VERSION(3, 0, 0)
	GdkRGBA color;
	double d;
#else
	GdkColor color;
	double d;
#endif

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, argc);
#endif
	if(argc == 3)
	{
#if GTK_CHECK_VERSION(3, 0, 0)
		color.red = (sscanf(argv[0], "%lf", &d) == 1) ? d / 255.0 : 0.0;
		color.green = (sscanf(argv[1], "%lf", &d) == 1)
			? d / 255.0 : 0.0;
		color.blue = (sscanf(argv[2], "%lf", &d) == 1)
			? d / 255.0 : 0.0;
		color.alpha = 1.0;
#else
		color.red = (sscanf(argv[0], "%lf", &d) == 1) ? d * 256 : 0;
		color.green = (sscanf(argv[1], "%lf", &d) == 1) ? d * 256 : 0;
		color.blue = (sscanf(argv[2], "%lf", &d) == 1) ? d * 256 : 0;
#endif
	}
	else if(argc != 0)
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
#if GTK_CHECK_VERSION(3, 4, 0)
	widget = gtk_color_chooser_widget_new();
	gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(widget), FALSE);
	if(argc == 3)
		gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(widget), &color);
#else
	widget = gtk_color_selection_new();
	gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(widget),
			FALSE);
	if(argc == 3)
# if GTK_CHECK_VERSION(3, 0, 0)
		gtk_color_selection_set_current_rgba(
				GTK_COLOR_SELECTION(widget), &color);
# else
		gtk_color_selection_set_current_color(
				GTK_COLOR_SELECTION(widget), &color);
# endif
#endif
	gtk_widget_show(widget);
	gtk_container_add(GTK_CONTAINER(container), widget);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
#if GTK_CHECK_VERSION(3, 4, 0)
			gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget),
					&color);
#elif GTK_CHECK_VERSION(3, 0, 0)
			gtk_color_selection_get_current_rgba(
					GTK_COLOR_SELECTION(widget), &color);
#else
			gtk_color_selection_get_current_color(
					GTK_COLOR_SELECTION(widget), &color);
#endif
#if GTK_CHECK_VERSION(3, 0, 0)
			dprintf(opt->output_fd, "%.0f %.0f %.0f\n",
					color.red * 255.0,
					color.green * 255.0,
					color.blue * 255.0);
#else
			dprintf(opt->output_fd, "%u %u %u\n",
					color.red / 256,
					color.green / 256,
					color.blue / 256);
#endif
			break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}


/* builder_combobox */
int builder_combobox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * widget;
	int i;
	gchar * p;

	dialog = _builder_dialog(conf, opt, text, rows, cols);
	_builder_dialog_buttons(dialog, conf, opt);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
	/* XXX this requires Gtk+ 2.24 */
	if(opt->editable)
		widget = gtk_combo_box_text_new_with_entry();
	else
		widget = gtk_combo_box_text_new();
	for(i = 0; i < argc; i++)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget),
				argv[i]);
	gtk_widget_show(widget);
	gtk_container_add(GTK_CONTAINER(container), widget);
	ret = _builder_dialog_run(conf, dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			p = gtk_combo_box_text_get_active_text(
					GTK_COMBO_BOX_TEXT(widget));
			if(p == NULL)
				break;
			dprintf(opt->output_fd, "%s\n", p);
			g_free(p);
			break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}


/* builder_dselect */
int builder_dselect(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	return _builder_dialog_fselect(conf, text, rows, cols, argc, argv, opt,
			GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
}


/* builder_editbox */
static void _editbox_print(struct options const * opt, GtkTextBuffer * buffer);

int builder_editbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	/* XXX copy/pasted from src/builders.c */
	int ret;
	struct textbox_data td;
	GtkWidget * container;
	GtkWidget * window;
	PangoFontDescription * desc = NULL;

	if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	td.opt = opt;
	td.editable = TRUE;
	td.scroll = FALSE;
	td.button = NULL;
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
	if(opt->fixed_font)
	{
		desc = pango_font_description_from_string("Monospace");
#if GTK_CHECK_VERSION(3, 0, 0)
		gtk_widget_override_font(td.view, desc);
#else
		gtk_widget_modify_font(td.view, desc);
#endif
	}
	td.buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(td.view));
	gtk_container_add(GTK_CONTAINER(window), td.view);
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 0);
	gtk_widget_show_all(window);
	if(!opt->without_buttons)
	{
#if GTK_CHECK_VERSION(2, 10, 0)
		if(opt->print != NULL)
		{
# if GTK_CHECK_VERSION(3, 12, 0)
			container = gtk_dialog_get_header_bar(
					GTK_DIALOG(td.dialog));
			if(container == NULL)
# endif
# if GTK_CHECK_VERSION(2, 14, 0)
				container = gtk_dialog_get_action_area(
						GTK_DIALOG(td.dialog));
# else
			container = td.dialog->action_area;
# endif
# if GTK_CHECK_VERSION(3, 10, 0)
			td.button = gtk_button_new_with_label("Print");
# else
			td.button = gtk_button_new_from_stock(GTK_STOCK_PRINT);
# endif
			g_signal_connect_swapped(td.button, "clicked",
					G_CALLBACK(_textbox_on_print), &td);
			gtk_widget_show(td.button);
			gtk_container_add(GTK_CONTAINER(container), td.button);
		}
#endif
		if(conf->button.without_cancel != true
				&& opt->high_compat == false)
			gtk_dialog_add_button(GTK_DIALOG(td.dialog), "Cancel",
					GTK_RESPONSE_CANCEL);
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
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			_editbox_print(opt, td.buffer);
			break;
	}
	gtk_widget_destroy(td.dialog);
	if(desc != NULL)
		pango_font_description_free(desc);
	return ret;
}

static void _editbox_print(struct options const * opt, GtkTextBuffer * buffer)
{
	GtkTextIter start, end;
	gchar * p;

	/* XXX this is a naive and potentially inefficient approach */
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	p = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	dprintf(opt->output_fd, "%s\n", p);
	g_free(p);
}


/* builder_fontsel */
int builder_fontsel(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * widget;
	gchar * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, argc);
#endif
	if(argc != 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, opt, NULL, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
#if GTK_CHECK_VERSION(3, 2, 0)
	widget = gtk_font_chooser_widget_new();
	gtk_font_chooser_set_font(GTK_FONT_CHOOSER(widget), text);
#else
	widget = gtk_font_selection_new();
	gtk_font_selection_set_font_name(GTK_FONT_SELECTION(widget), text);
#endif
	gtk_widget_show(widget);
	gtk_container_add(GTK_CONTAINER(container), widget);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
#if GTK_CHECK_VERSION(3, 2, 0)
			p = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(widget));
#else
			p = gtk_font_selection_get_font_name(
					GTK_FONT_SELECTION(widget));
#endif
			dprintf(opt->output_fd, "%s\n", p);
			g_free(p);
			break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}


/* builder_fselect */
int builder_fselect(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	return _builder_dialog_fselect(conf, text, rows, cols, argc, argv, opt,
			GTK_FILE_CHOOSER_ACTION_OPEN);
}


/* builder_logbox */
static gboolean _logbox_on_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data);
static gboolean _logbox_on_can_read_eof(gpointer data);
static gboolean _logbox_on_idle(gpointer data);

int builder_logbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	struct logbox_data ld;
	GtkWidget * container;
	GtkWidget * window;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	ld.opt = opt;
	ld.filename = text;
	ld.dialog = _builder_dialog(conf, opt, NULL, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(ld.dialog));
#else
	container = td.dialog->vbox;
#endif
	ld.store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	if(conf->shadow == false)
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(window),
				GTK_SHADOW_NONE);
	ld.view = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(ld.view), TRUE);
	gtk_tree_view_set_model(GTK_TREE_VIEW(ld.view),
			GTK_TREE_MODEL(ld.store));
	renderer = gtk_cell_renderer_text_new();
	if(opt->fixed_font)
		g_object_set(renderer, "family", "Monospace", NULL);
	column = gtk_tree_view_column_new_with_attributes("Date - Time",
			renderer, "text", 0, NULL);
	gtk_tree_view_column_set_expand(column, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ld.view), column);
	renderer = gtk_cell_renderer_text_new();
	if(opt->fixed_font)
		g_object_set(renderer, "family", "Monospace", NULL);
	column = gtk_tree_view_column_new_with_attributes("Log message",
			renderer, "text", 1, NULL);
	gtk_tree_view_column_set_expand(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ld.view), column);
	gtk_container_add(GTK_CONTAINER(window), ld.view);
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 0);
	gtk_widget_show_all(window);
	if(!opt->without_buttons)
		_builder_dialog_buttons(ld.dialog, conf, opt);
#if GTK_CHECK_VERSION(3, 12, 0)
	if((container = gtk_dialog_get_header_bar(GTK_DIALOG(ld.dialog)))
			!= NULL)
		gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(container),
				FALSE);
#endif
	ld.id = g_idle_add(_logbox_on_idle, &ld);
	/* FIXME also monitor changes to the file */
	ret = _builder_dialog_run(conf, ld.dialog);
	if(ld.id != 0)
		g_source_remove(ld.id);
	gtk_widget_destroy(ld.dialog);
	return ret;
}

static gboolean _logbox_on_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data)
{
	struct logbox_data * ld = data;
	GIOStatus status;
	gchar * line;
	gsize r;
	GError * error = NULL;
	GtkTreeIter iter;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(condition != G_IO_IN)
	{
		_builder_dialog_error(ld->dialog, NULL, NULL,
				"Unexpected condition");
		return _logbox_on_can_read_eof(ld);
	}
	status = g_io_channel_read_line(channel, &line, NULL, &r, &error);
	if(status == G_IO_STATUS_ERROR)
	{
		_builder_dialog_error(ld->dialog, NULL, NULL, error->message);
		g_error_free(error);
		return _logbox_on_can_read_eof(ld);
	}
	else if(status == G_IO_STATUS_AGAIN)
		return TRUE;
	else if(status == G_IO_STATUS_EOF)
		return _logbox_on_can_read_eof(ld);
	line[r] = '\0';
	if(ld->opt->reverse)
		gtk_list_store_insert(GTK_LIST_STORE(ld->store), &iter, 0);
	else
		gtk_list_store_append(GTK_LIST_STORE(ld->store), &iter);
	/* FIXME determine the timestamp */
	gtk_list_store_set(GTK_LIST_STORE(ld->store), &iter, 0, "", 1, line,
			-1);
	g_free(line);
	return TRUE;
}

static gboolean _logbox_on_can_read_eof(gpointer data)
{
	struct logbox_data * ld = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ld->id = 0;
	return FALSE;
}

static gboolean _logbox_on_idle(gpointer data)
{
	struct logbox_data * ld = data;
	char buf[BUFSIZ];
	gboolean close = TRUE;

	ld->id = 0;
	if(strcmp(ld->filename, "-") == 0)
	{
		ld->fd = STDIN_FILENO;
		close = FALSE;
	}
	else if((ld->fd = open(ld->filename, O_RDONLY)) <= -1)
	{
		snprintf(buf, sizeof(buf), "%s: %s", ld->filename,
				strerror(errno));
		_builder_dialog_error(ld->dialog, NULL, NULL, buf);
		gtk_dialog_response(GTK_DIALOG(ld->dialog), BSDDIALOG_ERROR);
		ld->id = 0;
		return FALSE;
	}
	ld->channel = g_io_channel_unix_new(ld->fd);
	g_io_channel_set_close_on_unref(ld->channel, close);
	g_io_channel_set_encoding(ld->channel, NULL, NULL);
	/* XXX ignore errors */
	g_io_channel_set_flags(ld->channel, g_io_channel_get_flags(ld->channel)
			| G_IO_FLAG_NONBLOCK, NULL);
	ld->id = g_io_add_watch(ld->channel, G_IO_IN, _logbox_on_can_read, ld);
	return FALSE;
}


/* builder_progress */
static gboolean _progress_on_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data);
static void _progress_on_can_read_append(struct progress_data * pd, char * buf,
		gsize * len);
static gboolean _progress_on_can_read_eof(gpointer data);
static void _progress_on_can_read_skip(struct progress_data * pd, char * buf,
		gsize * len);
static void _progress_set_percentage(struct progress_data * pd,
		unsigned int perc);

int builder_progress(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	int ret;
	struct progress_data pd = { NULL, NULL, NULL, NULL, 0, 0, 0, 0 };
	GtkWidget * container;
	GIOChannel * channel;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, argc);
#endif
	if(argc > 2)
	{
		error_args(opt->name, argc - 1, argv + 1);
		return BSDDIALOG_ERROR;
	}
	if(argc == 2)
		pd.msglen = strtol(argv[1], NULL, 10);
	if(argc >= 1)
		pd.maxdots = strtoul(argv[0], NULL, 10);
	pd.opt = opt;
	pd.dialog = _builder_dialog(conf, opt, NULL, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(pd.dialog));
#else
	container = pd.dialog->vbox;
#endif
	if(text != NULL)
	{
		pd.label = gtk_label_new(text);
		gtk_label_set_line_wrap(GTK_LABEL(pd.label), TRUE);
		gtk_label_set_line_wrap_mode(GTK_LABEL(pd.label),
				PANGO_WRAP_WORD_CHAR);
		gtk_label_set_single_line_mode(GTK_LABEL(pd.label), FALSE);
#if GTK_CHECK_VERSION(3, 10, 0)
		if(rows > 0)
			gtk_label_set_lines(GTK_LABEL(pd.label), rows);
#endif
#if GTK_CHECK_VERSION(3, 14, 0)
		gtk_widget_set_halign(pd.label, opt->halign);
#else
		gtk_misc_set_alignment(GTK_MISC(pd.label), opt->halign, 0.5);
#endif
#ifdef WITH_XDIALOG
		gtk_label_set_justify(GTK_LABEL(pd.label), opt->justify);
#endif
		gtk_widget_show(pd.label);
		gtk_box_pack_start(GTK_BOX(container), pd.label, FALSE, TRUE,
				BORDER_WIDTH);
	}
	pd.widget = gtk_progress_bar_new();
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(pd.widget), TRUE);
#endif
	_progress_set_percentage(&pd, 0);
	gtk_widget_show(pd.widget);
	gtk_container_add(GTK_CONTAINER(container), pd.widget);
	channel = g_io_channel_unix_new(STDIN_FILENO);
	g_io_channel_set_encoding(channel, NULL, NULL);
	/* XXX ignore errors */
	g_io_channel_set_flags(channel, g_io_channel_get_flags(channel)
			| G_IO_FLAG_NONBLOCK, NULL);
	pd.id = g_io_add_watch(channel, G_IO_IN, _progress_on_can_read, &pd);
	ret = _builder_dialog_run(conf, pd.dialog);
	if(pd.id != 0)
		g_source_remove(pd.id);
	gtk_widget_destroy(pd.dialog);
	return ret;
}

static gboolean _progress_on_can_read(GIOChannel * channel,
		GIOCondition condition, gpointer data)
{
	struct progress_data * pd = data;
	GIOStatus status;
	char buf[BUFSIZ + 1];
	gsize r;
	GError * error = NULL;
	char * p;
	unsigned int perc;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(condition != G_IO_IN)
	{
		_builder_dialog_error(pd->dialog, NULL, NULL,
				"Unexpected condition");
		return _progress_on_can_read_eof(pd);
	}
	if((status = g_io_channel_read_chars(channel, buf, sizeof(buf) - 1,
					&r, &error)) == G_IO_STATUS_ERROR)
	{
		_builder_dialog_error(pd->dialog, NULL, NULL, error->message);
		g_error_free(error);
		return _progress_on_can_read_eof(pd);
	}
	else if(status == G_IO_STATUS_AGAIN)
		return TRUE;
	else if(status == G_IO_STATUS_EOF)
		return _progress_on_can_read_eof(pd);
	buf[r] = '\0';
	_progress_on_can_read_skip(pd, buf, &r);
	_progress_on_can_read_append(pd, buf, &r);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() buf=\"%s\"\n", __func__, buf);
#endif
	for(p = buf; p[0] != '\0'; p++)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, p);
#endif
		if(sscanf(p, "%u", &perc) == 1)
			/* set the current percentage */
			_progress_set_percentage(pd, perc);
		else
		{
			pd->count += r;
			_progress_set_percentage(pd, pd->count);
			break;
		}
		if((p = strchr(p, '\n')) == NULL)
			break;
	}
	return TRUE;
}

static void _progress_on_can_read_append(struct progress_data * pd, char * buf,
		gsize * len)
{
	gsize max;
	char c;

	if(pd->msglen <= 0)
		return;
	max = (*len > (gsize)pd->msglen) ? pd->msglen : *len;
	/* set the current text */
	c = buf[max];
	buf[max] = '\0';
	/* FIXME append to the text instead */
	gtk_label_set_text(GTK_LABEL(pd->label), buf);
	buf[max] = c;
	*len -= max;
	memmove(buf, &buf[max], *len + 1);
	pd->msglen -= max;
}

static gboolean _progress_on_can_read_eof(gpointer data)
{
	struct progress_data * pd = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(!pd->opt->ignore_eof)
		gtk_dialog_response(GTK_DIALOG(pd->dialog), GTK_RESPONSE_CLOSE);
	pd->id = 0;
	return FALSE;
}

static void _progress_on_can_read_skip(struct progress_data * pd, char * buf,
		gsize * len)
{
	gsize max;

	if(pd->msglen >= 0)
		return;
	max = (*len > (gsize)-pd->msglen) ? -pd->msglen : *len;
	*len -= max;
	memmove(buf, &buf[max], *len + 1);
	pd->msglen += max;
}

static void _progress_set_percentage(struct progress_data * pd,
		unsigned int perc)
{
	gdouble fraction;
	char buf[8];

	perc = MIN(perc, pd->maxdots);
	fraction = (gdouble)perc / (gdouble)pd->maxdots;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pd->widget), fraction);
	snprintf(buf, sizeof(buf), "%.0f %%", fraction * 100);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pd->widget), buf);
}


/* builder_tailbox */
int builder_tailbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	/* XXX copy/pasted from src/builders.c */
	int ret;
	struct textbox_data td;
	GtkWidget * container;
	GtkWidget * window;
	PangoFontDescription * desc = NULL;

	if(argc > 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	td.opt = opt;
	td.editable = FALSE;
	td.scroll = TRUE;
	td.button = NULL;
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
	if(opt->fixed_font)
	{
		desc = pango_font_description_from_string("Monospace");
#if GTK_CHECK_VERSION(3, 0, 0)
		gtk_widget_override_font(td.view, desc);
#else
		gtk_widget_modify_font(td.view, desc);
#endif
	}
	td.buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(td.view));
	gtk_container_add(GTK_CONTAINER(window), td.view);
	gtk_box_pack_start(GTK_BOX(container), window, TRUE, TRUE, 0);
	gtk_widget_show_all(window);
	if(!opt->without_buttons)
	{
#if GTK_CHECK_VERSION(2, 10, 0)
		if(opt->print != NULL)
		{
# if GTK_CHECK_VERSION(3, 12, 0)
			container = gtk_dialog_get_header_bar(
					GTK_DIALOG(td.dialog));
			if(container == NULL)
# endif
# if GTK_CHECK_VERSION(2, 14, 0)
				container = gtk_dialog_get_action_area(
						GTK_DIALOG(td.dialog));
# else
			container = td.dialog->action_area;
# endif
# if GTK_CHECK_VERSION(3, 10, 0)
			td.button = gtk_button_new_with_label("Print");
# else
			td.button = gtk_button_new_from_stock(GTK_STOCK_PRINT);
# endif
			g_signal_connect_swapped(td.button, "clicked",
					G_CALLBACK(_textbox_on_print), &td);
			gtk_widget_show(td.button);
			gtk_container_add(GTK_CONTAINER(container), td.button);
		}
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
	/* FIXME also monitor changes to the file */
	ret = _builder_dialog_run(conf, td.dialog);
	if(td.id != 0)
		g_source_remove(td.id);
	gtk_widget_destroy(td.dialog);
	if(desc != NULL)
		pango_font_description_free(desc);
	return ret;
}


/* private */
/* functions */
/* builder_dialog_fselect */
static int _builder_dialog_fselect(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt,
		GtkFileChooserAction action)
{
	int ret;
	GtkWidget * dialog;
	GtkWidget * container;
	GtkWidget * widget;
	gchar * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, argc);
#endif
	if(argc != 0)
	{
		error_args(opt->name, argc, argv);
		return BSDDIALOG_ERROR;
	}
	dialog = _builder_dialog(conf, opt, NULL, rows, cols);
#if GTK_CHECK_VERSION(2, 14, 0)
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	container = dialog->vbox;
#endif
	widget = gtk_file_chooser_widget_new(action);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widget), text);
	gtk_widget_show(widget);
	gtk_container_add(GTK_CONTAINER(container), widget);
	_builder_dialog_buttons(dialog, conf, opt);
	ret = _builder_dialog_run(conf, dialog);
	switch(ret)
	{
		case BSDDIALOG_EXTRA:
		case BSDDIALOG_OK:
			p = gtk_file_chooser_get_filename(
					GTK_FILE_CHOOSER(widget));
			dprintf(opt->output_fd, "%s\n", (p != NULL) ? p : "");
			g_free(p);
			break;
	}
	gtk_widget_destroy(dialog);
	return ret;
}
