EmEditor Whitespace Plug-in
---------------------------

This plug-in offers a more complete alternative to the "ShowReturns" and "Trim
Right" plug-ins. When its icon is selected from the plug-in toolbar, a pop-up
menu is displayed offering to

    - Directly enable / disable the display of whitespace characters:

        * Returns
        * End of File
        * Tabs
        * Spaces

    - Convert leading Spaces to Tabs (Tabify)

    - Convert Tabs to Spaces (Untabify)

    - Trim Trailing Whitespaces

All the conversion commands work on the current selection, or the whole document
if there is no selection. In this they differ from the built-in commands
available from the "Edit" main menu.

Additionally, opened files are analyzed for their line endings and the detected
style is shown in the status bar. Supported styles are DOS, UNIX, MAC and Mixed.
Due to plug-in API restrictions, only the currently active document is analyzed
at a time.

The "pilcrow" icon was taken from the Silk Icon Set 1.3 freely available at
http://www.famfamfam.com/lab/icons/silk/

Sebastian Schuberth <sschuberth@gmail.com>
