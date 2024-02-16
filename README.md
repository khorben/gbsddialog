gbsddialog
==========

This is an implementation of [bsddialog](https://gitlab.com/alfix/bsddialog), a
text-based user interface to be used in scripts and tools, such as in the
official [FreeBSD](https://www.FreeBSD.org) installer, `bsdinstall(8)`.

However, gbsddialog uses [Gtk+](https://gtk.org/) dialogs and widgets instead of
console-based equivalents.

The objective is to reproduce the same behaviour as the original tool,
respecting the exact command-line parameters and output. gbsddialog can then be
used as a drop-in replacement to bsddialog, effectively granting the possibility
to install and configure FreeBSD entirely in graphical mode.

In addition, an implementation of [Xdialog](http://xdialog.free.fr) is also
provided, for use with `bsdconfig(8)` when invoked in graphical mode with `-S`
or `-X`.

This project is sponsored by the FreeBSD Foundation.

Dependencies
------------

gbsdinstall only depends on Gtk+. While it is meant to be used with version 3
of Gtk+, it can also be built with version 2. (Ideally version 2.10 or later)

To switch between version 2 or 3, modify `src/Makefile` and `tools/Makefile`
where indicated.

Development Plan
----------------

This project is currently being implemented in fast pace, in order to provide an
early preview and prototype for a graphical version of the FreeBSD installer.

This involves:

- A [fork of bsdinstall](https://github.com/khorben/bsdinstall), with the added
  flexibility of replacing the tool used for widgets and dialogs, as well as
  additional ways to test the code without possibly harming the host system.
- The ability to fallback to the original bsddialog in situations where
  gbsdinstall does not support a feature yet.

Different phases are planned, as follows:

1. Basic implementation, possibly ignoring some options.
1. Implementation with a desktop window. (for e.g., `--backtitle`)
1. Support for a daemon mode, where the desktop window remains active, in order
   to avoid flickering between invocations.
1. Additional improvement where the main dialog window is re-filled, instead of
   new dialogs popping up for each step.
1. Implementation using the GtkAssistant widget.

Known Issues
------------

The current status according to the development plan above is at step 2.
"Implementation with a desktop window", whereas support for a desktop window is
implemented. However:

- A few features of `bsddialog(1)` are still missing, such as highlights for
  text or generic buttons.
- Some features of `Xdialog(1)` are still missing or incomplete, like tracking
  changes to files in logbox and tailbox.
- Depending on the version of Gtk+ installed, some of the API calls used may be
  considered obsolete during the build. (Triggering many compilation warnings)

