/* gbsddialog */
/* gbsddialog.c */



#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include "builders.h"
#include "common.h"
#include "gbsddialog.h"

/* FIXME conflicts with <sys/syslimits.h> */
#undef MAX_INPUT


/* getopt_long */
enum OPTS {
	/* Options */
	ALTERNATE_SCREEN = '?' + 1,
	AND_DIALOG,
	ASCII_LINES,
	BACKTITLE,
	BEGIN_X,
	BEGIN_Y,
	BIKESHED,
	CANCEL_EXIT_CODE,
	CANCEL_LABEL,
	CLEAR_DIALOG,
	CLEAR_SCREEN,
	COLUMNS_PER_ROW,
	CR_WRAP,
	DATEBOX_FORMAT,
	DATE_FORMAT,
	DEFAULT_BUTTON,
	DEFAULT_ITEM,
	DEFAULT_NO,
	DISABLE_ESC,
	ERROR_EXIT_CODE,
	ESC_EXIT_CODE,
	EXIT_LABEL,
	EXTRA_BUTTON,
	EXTRA_EXIT_CODE,
	EXTRA_LABEL,
	HELP_BUTTON,
	HELP_EXIT_CODE,
	HELP_LABEL,
	HELP_PRINT_ITEMS,
	HELP_PRINT_NAME,
	HFILE,
	HLINE,
	HMSG,
	IGNORE,
	INSECURE,
	ITEM_BOTTOM_DESC,
	ITEM_DEPTH,
	ITEM_PREFIX,
	LEFT1_BUTTON,
	LEFT1_EXIT_CODE,
	LEFT2_BUTTON,
	LEFT2_EXIT_CODE,
	LEFT3_BUTTON,
	LEFT3_EXIT_CODE,
	LOAD_THEME,
	MAX_INPUT,
	NO_CANCEL,
	NO_DESCRIPTIONS,
	NO_LINES,
	NO_NAMES,
	NO_OK,
	NO_SHADOW,
	NORMAL_SCREEN,
	OK_EXIT_CODE,
	OK_LABEL,
	OUTPUT_FD,
	OUTPUT_SEPARATOR,
	PRINT_MAXSIZE,
	PRINT_SIZE,
	PRINT_VERSION,
	QUOTED,
	RIGHT1_BUTTON,
	RIGHT1_EXIT_CODE,
	RIGHT2_BUTTON,
	RIGHT2_EXIT_CODE,
	RIGHT3_BUTTON,
	RIGHT3_EXIT_CODE,
	SAVE_THEME,
	SEPARATE_OUTPUT,
	SHADOW,
	SINGLE_QUOTED,
	SLEEP,
	STDERR,
	STDOUT,
	SWITCH_BUTTONS,
	TAB_ESCAPE,
	TAB_LEN,
	TEXT_ESCAPE,
	TEXT_UNCHANGED,
	THEME,
	TIMEOUT_EXIT_CODE,
	TIME_FORMAT,
	TITLE,
	/* Dialogs */
	CALENDAR,
	CHECKLIST,
	DATEBOX,
	FORM,
	GAUGE,
	INFOBOX,
	INPUTBOX,
	MENU,
	MIXEDFORM,
	MIXEDGAUGE,
	MSGBOX,
	PASSWORDBOX,
	PASSWORDFORM,
	PAUSE,
	RADIOLIST,
	RANGEBOX,
	TEXTBOX,
	TIMEBOX,
	TREEVIEW,
	YESNO
};

static struct option longopts[] = {
	/* Options */
#if 0
	{"alternate-screen",  no_argument,       NULL, ALTERNATE_SCREEN},
	{"and-dialog",        no_argument,       NULL, AND_DIALOG},
	{"and-widget",        no_argument,       NULL, AND_DIALOG},
	{"ascii-lines",       no_argument,       NULL, ASCII_LINES},
	{"backtitle",         required_argument, NULL, BACKTITLE},
	{"begin-x",           required_argument, NULL, BEGIN_X},
	{"begin-y",           required_argument, NULL, BEGIN_Y},
	{"bikeshed",          no_argument,       NULL, BIKESHED},
	{"cancel-exit-code",  required_argument, NULL, CANCEL_EXIT_CODE},
	{"cancel-label",      required_argument, NULL, CANCEL_LABEL},
	{"clear",             no_argument,       NULL, CLEAR_SCREEN},
	{"clear-dialog",      no_argument,       NULL, CLEAR_DIALOG},
	{"clear-screen",      no_argument,       NULL, CLEAR_SCREEN},
	{"colors",            no_argument,       NULL, TEXT_ESCAPE},
	{"columns-per-row",   required_argument, NULL, COLUMNS_PER_ROW},
	{"cr-wrap",           no_argument,       NULL, CR_WRAP},
	{"datebox-format",    required_argument, NULL, DATEBOX_FORMAT},
	{"date-format",       required_argument, NULL, DATE_FORMAT},
	{"defaultno",         no_argument,       NULL, DEFAULT_NO},
	{"default-button",    required_argument, NULL, DEFAULT_BUTTON},
	{"default-item",      required_argument, NULL, DEFAULT_ITEM},
	{"default-no",        no_argument,       NULL, DEFAULT_NO},
	{"disable-esc",       no_argument,       NULL, DISABLE_ESC},
	{"error-exit-code",   required_argument, NULL, ERROR_EXIT_CODE},
	{"esc-exit-code",     required_argument, NULL, ESC_EXIT_CODE},
	{"exit-label",        required_argument, NULL, EXIT_LABEL},
	{"extra-button",      no_argument,       NULL, EXTRA_BUTTON},
	{"extra-exit-code",   required_argument, NULL, EXTRA_EXIT_CODE},
	{"extra-label",       required_argument, NULL, EXTRA_LABEL},
#endif
	{"help-button",       no_argument,       NULL, HELP_BUTTON},
	{"help-exit-code",    required_argument, NULL, HELP_EXIT_CODE},
	{"help-label",        required_argument, NULL, HELP_LABEL},
#if 0
	{"help-print-items",  no_argument,       NULL, HELP_PRINT_ITEMS},
	{"help-print-name",   no_argument,       NULL, HELP_PRINT_NAME},
	{"help-status",       no_argument,       NULL, HELP_PRINT_ITEMS},
	{"help-tags",         no_argument,       NULL, HELP_PRINT_NAME},
	{"hfile",             required_argument, NULL, HFILE},
	{"hline",             required_argument, NULL, HLINE},
	{"hmsg",              required_argument, NULL, HMSG},
	{"ignore",            no_argument,       NULL, IGNORE},
	{"insecure",          no_argument,       NULL, INSECURE},
	{"item-bottom-desc",  no_argument,       NULL, ITEM_BOTTOM_DESC},
	{"item-depth",        no_argument,       NULL, ITEM_DEPTH},
	{"item-help",         no_argument,       NULL, ITEM_BOTTOM_DESC},
	{"item-prefix",       no_argument,       NULL, ITEM_PREFIX},
	{"keep-tite",         no_argument,       NULL, ALTERNATE_SCREEN},
	{"left1-button",      required_argument, NULL, LEFT1_BUTTON},
	{"left1-exit-code",   required_argument, NULL, LEFT1_EXIT_CODE},
	{"left2-button",      required_argument, NULL, LEFT2_BUTTON},
	{"left2-exit-code",   required_argument, NULL, LEFT2_EXIT_CODE},
	{"left3-button",      required_argument, NULL, LEFT3_BUTTON},
	{"left3-exit-code",   required_argument, NULL, LEFT3_EXIT_CODE},
	{"load-theme",        required_argument, NULL, LOAD_THEME},
	{"max-input",         required_argument, NULL, MAX_INPUT},
	{"no-cancel",         no_argument,       NULL, NO_CANCEL},
	{"nocancel",          no_argument,       NULL, NO_CANCEL},
	{"no-descriptions",   no_argument,       NULL, NO_DESCRIPTIONS},
	{"no-items",          no_argument,       NULL, NO_DESCRIPTIONS},
	{"no-label",          required_argument, NULL, CANCEL_LABEL},
	{"no-lines",          no_argument,       NULL, NO_LINES},
	{"no-names",          no_argument,       NULL, NO_NAMES},
#endif
	{"no-ok",             no_argument,       NULL, NO_OK},
	{"nook",              no_argument,       NULL, NO_OK},
#if 0
	{"no-shadow",         no_argument,       NULL, NO_SHADOW},
	{"no-tags",           no_argument,       NULL, NO_NAMES},
	{"normal-screen",     no_argument,       NULL, NORMAL_SCREEN},
#endif
	{"ok-exit-code",      required_argument, NULL, OK_EXIT_CODE},
	{"ok-label",          required_argument, NULL, OK_LABEL},
#if 0
	{"output-fd",         required_argument, NULL, OUTPUT_FD},
	{"output-separator",  required_argument, NULL, OUTPUT_SEPARATOR},
	{"print-maxsize",     no_argument,       NULL, PRINT_MAXSIZE},
	{"print-size",        no_argument,       NULL, PRINT_SIZE},
	{"print-version",     no_argument,       NULL, PRINT_VERSION},
	{"quoted",            no_argument,       NULL, QUOTED},
	{"right1-button",     required_argument, NULL, RIGHT1_BUTTON},
	{"right1-exit-code",  required_argument, NULL, RIGHT1_EXIT_CODE},
	{"right2-button",     required_argument, NULL, RIGHT2_BUTTON},
	{"right2-exit-code",  required_argument, NULL, RIGHT2_EXIT_CODE},
	{"right3-button",     required_argument, NULL, RIGHT3_BUTTON},
	{"right3-exit-code",  required_argument, NULL, RIGHT3_EXIT_CODE},
	{"save-theme",        required_argument, NULL, SAVE_THEME},
	{"separate-output",   no_argument,       NULL, SEPARATE_OUTPUT},
	{"separator",         required_argument, NULL, OUTPUT_SEPARATOR},
	{"shadow",            no_argument,       NULL, SHADOW},
	{"single-quoted",     no_argument,       NULL, SINGLE_QUOTED},
	{"sleep",             required_argument, NULL, SLEEP},
	{"stderr",            no_argument,       NULL, STDERR},
	{"stdout",            no_argument,       NULL, STDOUT},
	{"switch-buttons",    no_argument,       NULL, SWITCH_BUTTONS},
	{"tab-escape",        no_argument,       NULL, TAB_ESCAPE},
	{"tab-len",           required_argument, NULL, TAB_LEN},
	{"text-escape",       no_argument,       NULL, TEXT_ESCAPE},
	{"text-unchanged",    no_argument,       NULL, TEXT_UNCHANGED},
	{"theme",             required_argument, NULL, THEME},
	{"timeout-exit-code", required_argument, NULL, TIMEOUT_EXIT_CODE},
	{"time-format",       required_argument, NULL, TIME_FORMAT},
#endif
	{"title",             required_argument, NULL, TITLE},
	{"yes-label",         required_argument, NULL, OK_LABEL},
	/* Dialogs */
#if 0
	{"calendar",     no_argument, NULL, CALENDAR},
	{"checklist",    no_argument, NULL, CHECKLIST},
	{"datebox",      no_argument, NULL, DATEBOX},
	{"form",         no_argument, NULL, FORM},
	{"gauge",        no_argument, NULL, GAUGE},
	{"infobox",      no_argument, NULL, INFOBOX},
	{"inputbox",     no_argument, NULL, INPUTBOX},
	{"menu",         no_argument, NULL, MENU},
	{"mixedform",    no_argument, NULL, MIXEDFORM},
	{"mixedgauge",   no_argument, NULL, MIXEDGAUGE},
#endif
	{"msgbox",       no_argument, NULL, MSGBOX},
#if 0
	{"passwordbox",  no_argument, NULL, PASSWORDBOX},
	{"passwordform", no_argument, NULL, PASSWORDFORM},
	{"pause",        no_argument, NULL, PAUSE},
	{"radiolist",    no_argument, NULL, RADIOLIST},
	{"rangebox",     no_argument, NULL, RANGEBOX},
	{"textbox",      no_argument, NULL, TEXTBOX},
	{"timebox",      no_argument, NULL, TIMEBOX},
	{"treeview",     no_argument, NULL, TREEVIEW},
	{"yesno",        no_argument, NULL, YESNO},
#endif
	/* END */
	{ NULL, 0, NULL, 0}
};


/* types */
typedef struct _GBSDDialog
{
	int * ret;
	int argc;
	char const ** argv;
} GBSDDialog;

/* prototypes */
static int _parseargs(int argc, char const ** argv,
	       	struct bsddialog_conf * conf, struct options * opt);


/* gbsddialog */
static gboolean _gbsddialog_on_idle(gpointer data);
static gboolean _gbsddialog_on_idle_quit(gpointer data);

int gbsddialog(int * ret, int argc, char const ** argv)
{
	GBSDDialog * gbd;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, \"%s\")\n", __func__, argc, argv[0]);
#endif
	if((gbd = malloc(sizeof(*gbd))) == NULL)
		return -error(errno, "%s", strerror(errno));
	memset(gbd, 0, sizeof(*gbd));
	gbd->ret = ret;
	gbd->argc = argc;
	gbd->argv = argv;
	g_idle_add(_gbsddialog_on_idle, gbd);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => 0\n", __func__);
#endif
	return 0;
}

static gboolean _gbsddialog_on_idle(gpointer data)
{
	GBSDDialog * gbd = data;
	int parsed, argc, res;
	struct bsddialog_conf conf;
	struct options opt;
	char * text = NULL;
	int rows = BSDDIALOG_AUTOSIZE, cols = BSDDIALOG_AUTOSIZE;
	GtkWidget * container, * dialog, * widget = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(gbd->argc=%d gbd->argv=\"%s\")\n",
		       	__func__, gbd->argc,
		       	(gbd->argc > 0) ? gbd->argv[0] : "(null)");
#endif
	if((parsed = _parseargs(gbd->argc, gbd->argv, &conf, &opt)) <= 0)
		return _gbsddialog_on_idle_quit(gbd);
	argc = parsed - optind;
	gbd->argv += optind;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() argc=%d (optind=%d)\n",
		       	__func__, argc, optind);
#endif

	if(opt.mandatory_dialog && opt.dialogbuilder == NULL)
	{
		*gbd->ret = EXITCODE(error(BSDDIALOG_ERROR, "expected a --<dialog>"));
		return _gbsddialog_on_idle_quit(gbd);
	}
	if(opt.dialogbuilder != NULL)
	{
		if(argc != 3)
		{
			*gbd->ret = EXITCODE(error(BSDDIALOG_ERROR, "expected <text> <rows> <cols>"));
			return _gbsddialog_on_idle_quit(gbd);
		}
		if ((text = strdup(gbd->argv[0])) == NULL)
		{
			*gbd->ret = EXITCODE(error(BSDDIALOG_ERROR, "cannot allocate <text>"));
			return _gbsddialog_on_idle_quit(gbd);
		}
		rows = (int)strtol(gbd->argv[1], NULL, 10);
		cols = (int)strtol(gbd->argv[2], NULL, 10);

		widget = opt.dialogbuilder(&conf, text, rows, cols,
				argc - 3, gbd->argv + 3, &opt);
		free(text);

		argc += 3;
	}

	if(widget == NULL)
		/* FIXME report error */
		return _gbsddialog_on_idle_quit(gbd);

	dialog = gtk_dialog_new();
	if(conf.title != NULL)
		gtk_window_set_title(GTK_WINDOW(dialog), conf.title);
	container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_container_add(GTK_CONTAINER(container), widget);
	if(conf.button.without_ok != true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf.button.ok_label != NULL)
				? conf.button.ok_label : "OK", GTK_RESPONSE_OK);
	if(conf.button.with_help == true)
		gtk_dialog_add_button(GTK_DIALOG(dialog),
				(conf.button.help_label != NULL)
				? conf.button.help_label : "Help", GTK_RESPONSE_HELP);
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	switch(res)
	{
		case GTK_RESPONSE_OK:
			*gbd->ret = exitcodes[BSDDIALOG_OK + 1].value;
			break;
		case GTK_RESPONSE_HELP:
			*gbd->ret = exitcodes[BSDDIALOG_HELP + 1].value;
			break;
	}

	gbd->argc -= argc - 1;
	gbd->argv += argc - 1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => gbd->argc=%d gbd->argv=\"%s\"\n",
		       	__func__, gbd->argc,
			(gbd->argc > 0) ? gbd->argv[0] : "(null)");
#endif

	if(gbd->argc <= 0)
		return _gbsddialog_on_idle_quit(gbd);

	return TRUE;
}

static gboolean _gbsddialog_on_idle_quit(gpointer data)
{
	GBSDDialog * gbd = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	gtk_main_quit();
	free(gbd);
	return FALSE;
}


/* parseargs */
static int _parseargs(int argc, char const ** argv,
	       	struct bsddialog_conf * conf, struct options * opt)
{                       
	int arg, parsed, i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, argc);
#endif

	memset(conf, 0, sizeof(*conf));
	conf->y = BSDDIALOG_CENTER;
	conf->x = BSDDIALOG_CENTER;
	conf->shadow = true;
	conf->text.cols_per_row = DEFAULT_COLS_PER_ROW;
	conf->key.enable_esc = true;
	conf->button.always_active = true;

	memset(opt, 0, sizeof(*opt));
	opt->theme = -1;
	opt->output_fd = STDERR_FILENO;
	opt->max_input_form = 2048;
	opt->mandatory_dialog = true;

	for(i = 0; i < argc; i++)
       	{
		if(strcmp(argv[i], "--and-dialog") == 0
				|| strcmp(argv[i], "--and-widget") == 0)
	       	{
			argc = i + 1;
			break;
		}               
	}                           
	parsed = argc;  
	while((arg = getopt_long(argc, argv, "", longopts, NULL)) != -1)
       	{
		switch(arg)
		{
			/* Options */
			case HELP_BUTTON:
				conf->button.with_help = true;
				break;
			case HELP_EXIT_CODE:
				exitcodes[BSDDIALOG_HELP + 1].value = strtol(optarg, NULL, 10);
				break;
			case HELP_LABEL:
				conf->button.help_label = optarg;
				break;
			case NO_OK:
				conf->button.without_ok = true;
				break;
			case OK_EXIT_CODE:
				exitcodes[BSDDIALOG_OK + 1].value = strtol(optarg, NULL, 10);
				break;
			case OK_LABEL:
				conf->button.ok_label = optarg;
				break;
			case TITLE:
				conf->title = optarg;
				break;
			/* Dialogs */
			case MSGBOX:
				if(opt->dialogbuilder != NULL)
					return -error(BSDDIALOG_ERROR, "%s and --msgbox without "
							"--and-dialog", opt->name);
				opt->name = "--";
				opt->dialogbuilder = builder_msgbox;
				break;
			default: /* Error */
				if(opt->ignore == true)
					break;
				return -error(BSDDIALOG_ERROR, "--ignore to continue");
		}
	}

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => %d\n", __func__, optind);
#endif
	return argc;
}
