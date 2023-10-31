/* gbsddialog */
/* builders.h */



#ifndef GBSDDIALOG_BUILDERS_H
# define GBSDDIALOG_BUILDERS_H

# include <gtk/gtk.h>
# include "common.h"


/* builders */
int builder_infobox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt);
int builder_msgbox(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt);
int builder_pause(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt);
int builder_yesno(struct bsddialog_conf const * conf,
		char const * text, int rows, int cols,
		int argc, char const ** argv, struct options const * opt);

#endif /* !GBSDDIALOG_BUILDERS_H */
