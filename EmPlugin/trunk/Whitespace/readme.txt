EmEditor Whitespace Plug-in
---------------------------

This plug-in offers a more complete alternative to the "ShowReturns" and "Trim
Right" plug-ins. When its icon is selected from the plug-in toolbar, a pop-up
menu is displayed offering toggles to enable / disable the display of Return,
EOF, Tab or Space characters in the current document's configuration.

Additionally, menu entries to convert leading spaces to tabs (Tabify) or tabs to
spaces (Untabify) and to trim trailing whitespace characters from the end of
text lines are provided. All these command work on the current selection, or the
whole document in case of no selection. In this they differ from the built-in
commands available in the Edit main menu.

Last but not least, opened files are analyzed for their line end style(s) and
the "guessed" (if the file has not yet been analyzed completely) or detected
style is shown in the status bar. Supported styles are DOS, UNIX, MAC and Mixed.
Due to plug-in API restrictions, only the currently active document is analyzed
at a time.

The "pilcrow" icon was taken from the Silk Icon Set 1.3 freely available at
http://www.famfamfam.com/lab/icons/silk/

Sebastian Schuberth <sschuberth@gmail.com>
