/* gbsddialog */
/* common.h */



#ifndef GBSDDIALOG_COMMON_H
# define GBSDDIALOG_COMMON_H

# include <stdbool.h>
# include <bsddialog.h>


/* common */
/* macros */
# define PACKAGE	"gbsddialog"
# define VERSION	"0.0.0"

# ifndef PROGNAME
#  define PROGNAME	PACKAGE
# endif

# ifndef PREFIX
#  define PREFIX	"/usr/local"
# endif
# ifndef DATADIR
#  define DATADIR	PREFIX "/share"
# endif
# ifndef LOCALEDIR
#  define LOCALEDIR	DATADIR "/locale"
# endif

# define DEFAULT_COLS_PER_ROW	10

# define EXITCODE(retval)	(exitcodes[retval + 1].value)


/* types */
enum bsddialog_default_theme
{
	BSDDIALOG_THEME_3D,
	BSDDIALOG_THEME_BLACKWHITE,
	BSDDIALOG_THEME_FLAT
};

struct exitcode
{
	const char * name;
	int value;
};

struct options
{
	/* Menus options */
	bool item_always_quote;
	char *item_default;
	bool item_depth;
	char *item_output_sep;
	bool item_output_sepnl;
	bool item_prefix;
	bool item_singlequote;
	/* Menus and Forms options */
	bool help_print_item_name;
	bool help_print_items;
	bool item_bottomdesc;
	/* Forms options */
	int unsigned max_input_form;
	/* Date and Time options */
	char *date_fmt;
	char *time_fmt;
	/* General options */
	int getH;
	int getW;
	bool ignore;
	int output_fd;
	/* Text option */
	bool cr_wrap;
	bool tab_escape;
	bool text_unchanged;
	/* Theme and Screen options*/
	char *backtitle;
	bool bikeshed;
	enum bsddialog_default_theme theme;
	bool clearscreen;
	char *loadthemefile;
	char *savethemefile;
	const char *screen_mode;
	/* Dialog */
	bool mandatory_dialog;
	const char *name;
	int (*dialogbuilder)(struct bsddialog_conf const * conf,
	    char const * text, int rows, int cols,
	    int argc, char const ** argv, struct options const * opt);
};


/* variables */
extern struct exitcode exitcodes[];


/* functions */
int error(int ret, char const * format, ...);

#endif /* !GBSDDIALOG_COMMON_H */
