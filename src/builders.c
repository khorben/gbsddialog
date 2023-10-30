/* gbsddialog */
/* builders.c */



#include <gtk/gtk.h>
#include "callbacks.h"
#include "builders.h"


/* builders */
/* builder_msgbox */
GtkWidget * builder_msgbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt)
{
	GtkWidget * widget;

	if(argc > 0)
		error_args(opt->name, argc, argv);
	widget = gtk_label_new(text);
	gtk_widget_show(widget);
	return widget;
}
