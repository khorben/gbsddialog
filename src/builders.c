/* gbsddialog */
/* builders.c */



#include <gtk/gtk.h>
#include "callbacks.h"
#include "common.h"
#include "builders.h"


/* builders */
/* types */
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
static int _builder_dialog_output(struct bsddialog_conf const * conf, int res);
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
	_builder_dialog_buttons(dialog, conf);
	res = _builder_dialog_run(dialog);
	return _builder_dialog_output(conf, res);
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
	g_signal_connect_swapped(widget, "activate", conf->button.default_cancel
			? G_CALLBACK(_passwordbox_on_activate_cancel)
			: G_CALLBACK(_passwordbox_on_activate), dialog);
	gtk_widget_show(widget);
	gtk_container_add(GTK_CONTAINER(container), widget);
	_builder_dialog_buttons(dialog, conf);
	ret = _builder_dialog_run(dialog);
	switch(ret)
	{
		case BSDDIALOG_OK:
			printf("%s %s\n", conf->button.ok_label,
					gtk_entry_buffer_get_text(buffer));
			ret = exitcodes[BSDDIALOG_OK + 1].value;
			break;
		default:
			ret = _builder_dialog_output(conf, ret);
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
	if(pd.id != 0)
		g_source_remove(pd.id);
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
static int _builder_dialog_output(struct bsddialog_conf const * conf, int res)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, res);
#endif
	switch(res)
	{
		case BSDDIALOG_CANCEL:
			printf("%s\n", conf->button.cancel_label
					? conf->button.cancel_label
					: "Cancel");
			break;
		case BSDDIALOG_ESC:
			printf("%s\n", "[ESC]");
			break;
		case BSDDIALOG_EXTRA:
			printf("%s\n", conf->button.extra_label
					? conf->button.extra_label
					: "Extra");
			break;
		case BSDDIALOG_OK:
			printf("%s\n", conf->button.ok_label
					? conf->button.ok_label
					: "OK");
			break;
	}
	return exitcodes[res + 1].value;
}


/* builder_dialog_run */
static int _builder_dialog_run(GtkWidget * dialog)
{
	int res;

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
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
