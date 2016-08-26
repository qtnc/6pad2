%6pad++ command-line options

This document explains what you can specifiy on the command-line, and present is a list of options, which you can use to change the behavior of 6pad++.

# Specifying files to open
Simply by indicating file names, you can open multiple files at once, e.g. `6pad++ file1.txt file2.txt file3.txt`.

You can optionally indicate a line and column number where to go to, using colon syntax, e.g. `file:line` and `file:line:column`. The cursor will be placed at the position specified immediately upon file opening.
If a line number is specified but  no column, by default it will be column 1, i.e. the beginning of the line.

Example: `6pad++ file1.txt:123 file2.txt:123:45`.

# Using pipes

You can use pipes to redirect the output of a command to 6pad++, or to send a content typed in 6pad++ to another command.
Example: `dir | 6pad++`

# Command-line options

## /extension=name
Load the extension specified, as if the corresponding directive was present in the configuration file (6pad++.ini).

## /run=script.py
Run the specified script, immediately after the load of all extensions and the implicit initialization script (6pad++.py).

## /configfile=file.ini
Load the specified configuration file in place of the default one. You can type `/configfile=-` to open 6pad++ without any configuration file.

## /nacked
Load 6pad++ without any extension, i.e. ignore all extensions specified in the configuration file. Only extensions specified in the command line with `/extension` are loaded.

## /headless
Run 6pad++ in headless mode, i.e. no window is displayed on screen. This can be useful to batch process many files with a script, or perform other operations that can be run rather quickly and without needing any user interaction.
