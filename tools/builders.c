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
	gtk_widget_set_halign(widget, GTK_ALIGN_START);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
#endif
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
	buffer1 = gtk_entry_buffer_new(argv[1], -1);
	widget = gtk_entry_new_with_buffer(buffer1);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	if(opt->max_input_form > 0)
		gtk_entry_set_max_length(GTK_ENTRY(widget),
				opt->max_input_form);
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
	gtk_widget_set_halign(widget, GTK_ALIGN_START);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
#endif
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
	buffer2 = gtk_entry_buffer_new(argv[3], -1);
	widget = gtk_entry_new_with_buffer(buffer2);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	if(opt->max_input_form > 0)
		gtk_entry_set_max_length(GTK_ENTRY(widget),
				opt->max_input_form);
	if(conf->form.securech != '\0')
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
	gtk_widget_set_halign(widget1, GTK_ALIGN_START);
#else
	gtk_misc_set_alignment(GTK_MISC(widget1), 0.0, 0.5);
#endif
	gtk_box_pack_start(GTK_BOX(box), widget1, FALSE, TRUE, BORDER_WIDTH);
	/* XXX detect and report errors */
	min = strtol(argv[1], NULL, 10);
	max = strtol(argv[2], NULL, 10);
	value1 = strtol(argv[3], NULL, 10);
	widget1 = gtk_spin_button_new_with_range((gdouble)min, (gdouble)max,
			1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget1), (gdouble)value1);
	gtk_entry_set_activates_default(GTK_ENTRY(widget1), TRUE);
	gtk_box_pack_start(GTK_BOX(box), widget1, FALSE, TRUE, BORDER_WIDTH);
	/* range 2 */
	widget2 = gtk_label_new(argv[4]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget2, GTK_ALIGN_START);
#else
	gtk_misc_set_alignment(GTK_MISC(widget2), 0.0, 0.5);
#endif
	gtk_box_pack_start(GTK_BOX(box), widget2, FALSE, TRUE, BORDER_WIDTH);
	/* XXX detect and report errors */
	min = strtol(argv[5], NULL, 10);
	max = strtol(argv[6], NULL, 10);
	value2 = strtol(argv[7], NULL, 10);
	widget2 = gtk_spin_button_new_with_range((gdouble)min, (gdouble)max,
			1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget2), (gdouble)value2);
	gtk_entry_set_activates_default(GTK_ENTRY(widget2), TRUE);
	gtk_box_pack_start(GTK_BOX(box), widget2, FALSE, TRUE, BORDER_WIDTH);
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
	gtk_widget_set_halign(widget, GTK_ALIGN_START);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
#endif
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
	buffer1 = gtk_entry_buffer_new(argv[1], -1);
	widget = gtk_entry_new_with_buffer(buffer1);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	if(opt->max_input_form > 0)
		gtk_entry_set_max_length(GTK_ENTRY(widget),
				opt->max_input_form);
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
	gtk_widget_set_halign(widget, GTK_ALIGN_START);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
#endif
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
	buffer2 = gtk_entry_buffer_new(argv[3], -1);
	widget = gtk_entry_new_with_buffer(buffer2);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	if(opt->max_input_form > 0)
		gtk_entry_set_max_length(GTK_ENTRY(widget),
				opt->max_input_form);
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
	gtk_widget_set_halign(widget, GTK_ALIGN_START);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
#endif
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, TRUE, 0);
	buffer3 = gtk_entry_buffer_new(argv[5], -1);
	widget = gtk_entry_new_with_buffer(buffer3);
	if(conf->button.always_active == true)
		gtk_entry_set_activates_default(GTK_ENTRY(widget), TRUE);
	if(opt->max_input_form > 0)
		gtk_entry_set_max_length(GTK_ENTRY(widget),
				opt->max_input_form);
	if(conf->form.securech != '\0')
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
	gtk_widget_set_halign(widget1, GTK_ALIGN_START);
#else
	gtk_misc_set_alignment(GTK_MISC(widget1), 0.0, 0.5);
#endif
	gtk_box_pack_start(GTK_BOX(box), widget1, FALSE, TRUE, BORDER_WIDTH);
	/* XXX detect and report errors */
	min = strtol(argv[1], NULL, 10);
	max = strtol(argv[2], NULL, 10);
	value1 = strtol(argv[3], NULL, 10);
	widget1 = gtk_spin_button_new_with_range((gdouble)min, (gdouble)max,
			1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget1), (gdouble)value1);
	gtk_entry_set_activates_default(GTK_ENTRY(widget1), TRUE);
	gtk_box_pack_start(GTK_BOX(box), widget1, FALSE, TRUE, BORDER_WIDTH);
	/* range 2 */
	widget2 = gtk_label_new(argv[4]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget2, GTK_ALIGN_START);
#else
	gtk_misc_set_alignment(GTK_MISC(widget2), 0.0, 0.5);
#endif
	gtk_box_pack_start(GTK_BOX(box), widget2, FALSE, TRUE, BORDER_WIDTH);
	/* XXX detect and report errors */
	min = strtol(argv[5], NULL, 10);
	max = strtol(argv[6], NULL, 10);
	value2 = strtol(argv[7], NULL, 10);
	widget2 = gtk_spin_button_new_with_range((gdouble)min, (gdouble)max,
			1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget2), (gdouble)value2);
	gtk_entry_set_activates_default(GTK_ENTRY(widget2), TRUE);
	gtk_box_pack_start(GTK_BOX(box), widget2, FALSE, TRUE, BORDER_WIDTH);
	/* range 3 */
	widget3 = gtk_label_new(argv[8]);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(widget3, GTK_ALIGN_START);
#else
	gtk_misc_set_alignment(GTK_MISC(widget3), 0.0, 0.5);
#endif
	gtk_box_pack_start(GTK_BOX(box), widget3, FALSE, TRUE, BORDER_WIDTH);
	/* XXX detect and report errors */
	min = strtol(argv[9], NULL, 10);
	max = strtol(argv[10], NULL, 10);
	value3 = strtol(argv[11], NULL, 10);
	widget3 = gtk_spin_button_new_with_range((gdouble)min, (gdouble)max,
			1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget3), (gdouble)value3);
	gtk_entry_set_activates_default(GTK_ENTRY(widget3), TRUE);
	gtk_box_pack_start(GTK_BOX(box), widget3, FALSE, TRUE, BORDER_WIDTH);
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
			GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER);
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
			GTK_FILE_CHOOSER_ACTION_SAVE);
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
