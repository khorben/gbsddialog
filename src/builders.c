/* gbsddialog */
/* builders.c */



#include <gtk/gtk.h>
#include "callbacks.h"
#include "common.h"
#include "builders.h"


/* builders */
/* prototypes */
static GtkWidget * _builder_dialog(struct bsddialog_conf const * conf,
		char const * text);
static int _builder_dialog_run(GtkWidget * dialog);


/* functions */
/* builder_infobox */
int builder_infobox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	GtkWidget * dialog;

	if(argc > 0)
		error_args(opt->name, argc, argv);
	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK, "%s", text);
	if(conf->title != NULL)
		gtk_window_set_title(GTK_WINDOW(dialog), conf->title);
	gtk_dialog_run(GTK_DIALOG(dialog));
	return 0;
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
	return _builder_dialog_run(dialog);
}


/* builder_yesno */
int builder_yesno(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	GtkWidget * dialog;
	int res;

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
	return _builder_dialog_run(dialog);
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


/* builder_dialog_run */
static int _builder_dialog_run(GtkWidget * dialog)
{
	int res;

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	switch(res)
	{
		case BSDDIALOG_EXTRA:
			return exitcodes[res + 1].value;
		case GTK_RESPONSE_CANCEL:
		case GTK_RESPONSE_NO:
			return exitcodes[BSDDIALOG_CANCEL + 1].value;
		case GTK_RESPONSE_HELP:
			return exitcodes[BSDDIALOG_HELP + 1].value;
		case GTK_RESPONSE_OK:
		case GTK_RESPONSE_YES:
			return exitcodes[BSDDIALOG_OK + 1].value;
	}
	return 0;
}
