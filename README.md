# About

This is a plug-in for the [EmEditor](http://www.emeditor.com/) text editor that simplifies dealing with whitespace characters.

If you want to show your appreciation and / or encourage future development, please consider donating via the button below (requires registration with the great [Flattr](https://flattr.com/) service).

[![Flattr this git repo](http://api.flattr.com/button/flattr-badge-large.png)](https://flattr.com/submit/auto?user_id=sschuberth&url=https://github.com/sschuberth/emwhitespace/&title=EmEditor%20Whitespace%20Plug-in&language=english&tags=github&category=software)

# Commands

This plug-in offers a more complete alternative to the "ShowReturns" and "Trim
Right" plug-ins. When its icon is selected from the plug-in toolbar, a pop-up
menu is displayed offering the following commands:

* "Show whitespaces"
  This toggles between the currently selected display options and displaying
  no whitespaces at all.

* "Show Line-Ends"
  Enable / disable showing line-end characters.

* "Show End of File"
  Enable / disable showing the end of the file.

* "Show Tabs"
  Enable / disable showing tabulator characters.

* "Show Spaces"
  Enable / disable showing space characters.

* "Ensure final Line-End"
  Makes sure the last line in the file has a line-end attached. Only is
  available for EmEditor 8.00 and above.

* "Spaces to Tabs (Tabify)"
  Replaces all leading spaces with tabulators according to the "Tab Columns"
  setting of the current configuration.

* "Tabs to Spaces (Untabify)"
  Replaces all tabulators with spaces according to the "Tab Columns" setting
  of the current configuration.

* "Trim Trailing Whitespaces"
  Removes all whitespaces from line ends.

All the conversion commands work on the current selection, or the whole document
if there is no selection. In this they differ from the built-in commands
available from the "Edit" main menu.

The "pilcrow" icon was taken from the Silk Icon Set 1.3 freely available at
http://www.famfamfam.com/lab/icons/silk/
