gbsddialog
==========

This is an implementation of [bsddialog](https://gitlab.com/alfix/bsddialog), a
text-based user interface to be used in scripts and tools, such as in the
official [FreeBSD](https://www.FreeBSD.org) installer, bsdinstall.

However, gbsddialog uses Gtk+ dialogs and widgets instead of console-based
equivalents.

The objective is to reproduce the same behaviour as the original tool,
respecting the exact command-line parameters and output. gbsddialog can then be
used as a drop-in replacement to bsddialog, effectively granting the possibility
to install FreeBSD entirely in graphical mode.

This project is sponsored by the FreeBSD Foundation.

Dependencies
------------

gbsdinstall only depends on [Gtk+](https://gtk.org/), in its version 3.

It can still easily be modified to work with Gtk+ version 2. (Ideally version
2.10 or later)

Development plan
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
1. Support for a daemon mode, where the desktop window remains active.
1. Additional improvement where the main dialog window is re-filled, instead of
   new dialogs popping up for each step.
1. Implementation using the GtkAssistant widget.

