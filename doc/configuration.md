%6pad++ configuration directives

Here is a list of configuration directives you can put on the configuration file.
The configuration file is always named after the 6pad++ executable file name except that .exe extension is replaced by .ini, and is always in the same directory.
I.e. if the executable of 6pad++ is called 6pad2.exe, the configuration will be in 6pad2.ini.

# Configuration directives

## extension
Define an extension to be loaded in 6pad++. This entry can be present multiple times to load several extensions.

- Specify a name without file extension to load the extension as a python module, i.e. import the given python module.
- Specify a file name ending in .py to execute the given python script, as if include were called.
- Specify a file name ending in .dll to load a c++ extension.

Note that the python file named after the 6pad++ executable is automatically run if it exists, i.e. if the 6pad++ executable is called 6pad2.exe, a file 6pad2.py is automatically executed at startup.

Python modules must be in *lib* or *plugins* directories in order to be importable. However, DLLs of C++ extensions must be in the same folder as the 6pad++ executable.

## defaultLineEnding {#le}
The default line ending convention to use when creating a new empty file. Default to 0.

0:
:	CRLF/windows
1:
:	LF/UNIX
2:
:	CR/Mac

## defaultEncoding {#ce}
The default encoding to use when creating a new empty file. Default to 1250-1258 depending on the current locale.

1252:
:	ANSI Windows 1252 aka. Latin1 Western Europe
65001:
:	UTF-8
65002:
:	UTF-8+BOM
1200:
:	UTF-16Le
1201:
:	UTF-16Be
1202:
:	UTF-16Le+BOM
1203:
:	UTF-16Be+BOM
28605:
:	ISO-8859-15

See <https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756(v=vs.85).aspx> for other values.

## defaultIndentationMode {#indent}
The default indentation convention to use when creating a new empty file. Default to 0 = tabs.

0:
:	Use tabs (`\t`) as indentation.
1-8:
:	Use 1 to 8 spaces as indentation

## defaultTabWidth
The default size, in spaces, visually taken by a tab characters, between 1 and 8. Default to 4.

## defaultAutoIndent {#defaultAutoIndent}
Define whether automatic indentation is on or off (true or false) for new files.

When automatic indentation is on, pressing enter creates a new line with the same indentation level as the previous line; when it is off, new lines always start without any indentation.

## defaultSmartPaste {#defaultSmartPaste}
Define whether smart paste is on or off (true or false) for new files.

When smart paste is on, indentations of the pasted text are striped away and readjusted to the indentation level of the current line. When it is off, no adjustment is done and the text is always pasted exactly as it has been stored in the clipboard.

## defaultSmartHome {#defaultSmartHome}
Define whether smart home is on or off (true or false) for new files.

When smart home is on, pressing *Home* place the cursor at the first non-blank character, after indentation spaces and tabs, instead of at the true beginning of the line.
Similarely, when there is no selection or if the selection doesn't spend multiple lines, pressing *Shift+Home* selects to the first non-blank character instead of the effective beginning of the line.
The normal selection / cursor movement to the effective beginning of the line behavior occurrs if this parameter is off, if the current selection spends multiple lines, or if you add the *Alt* key. In other words you can place the cursor at the true beginning of the line by pressing *Alt+Home*.

## defaultSafeIndent {#defaultSafeIndent}
Define whether safe indentation is on or off (true or false) for new files.

When safe indentation is on, the following behaviors occur:

* When pressing *Del* at the end of a line, the next line is joined to the current one as usual, but indentation of the joining line is triped away. The importance of this behavior is most notably useful when pressing *Del* at the end of a line and then again *Enter* to split it again. Without this adjustment, repressing *enter* may cause the indentation of the next line to double unexpectedly.
* Pressing *backspace* within indentation spaces or tabs has no effect, to make sure you don't erase indentation levels by accident. To remove indentation levels, *Backspace* only works if the cursor is on the first non-blank character. Note that you can also use *Shift+Tab* to remove indentation levels.

## instanceMode
Define how to manage multiple instances of the program and multiple files opened at once. Default to 0.

0:
:	No special preference. Allow multiple instances running at the same time, and allow loading multiple files in the same instances by using tabs.
1:
:	Always open all files in the same program instance. Disallow multiple instances running at the same time.
2:
:	Always open new files in new program instances. Never open more than one file in the same instance, and never use tabs.

## reloadLastFilesMode 
Define whether to reload files opened when leaving at last session, and when to do or not do it. Default to 0.

0:
:	Never reload files which were left open at last session.
1:
:	Reload files which were open at last session, but only if no other file to load is specified on the command line.
2:
:	Always reload all files opened at last session, even if other files given on the command line must be loaded.

## editorConfigOverride
Define how to behave with .editorconfig files, and how the settings defined in these files must override or not default parameters and/or guessed file formats.
.editorconfig files allow to locally define parameters such as encoding and line endings, so that all users working on a project can use the same format, regardless of specific IDEs configuration and/or project specific data.
For more information about .editorconfig files, see <http://editorconfig.org/>.
By default, this parameter is set to 1.

0:
:	Ignore completely .editorconfig files, never process them
1:
:	Use .editorconfig files, but load the file in the format guessed rather than how it is specified in .editorconfig if it appear to be more appropriate. When saving the file, settings defined in .editorconfig are always taken into account.
2:
:	Use .editorconfig files and always open the files in the format specified in .editorconfig, even if there are potential encoding and/or line endings conflicts.

The following standard .editorconfig properties are supported :

charset:
:	The character set encoding: "Latin1", "UTF8", "UTF8BOM", "UTF16Le", "UTF16Be"
end_of_line:
:	Line ending type: "CRLF", "LF" or "CR"
indent_style:
:	Indentation type: "tab" or "space"
indent_size:
:	Number of spaces of an indentation level when using spaces as indentation: number in range 1-8.
tab_width:
:	Size of a tab when using tabs as indentation: number in range 1-8

The following proprietary .editorconfig parameters are also recognized :

_6p_auto_line_break:
:	Define [defaultAutoLineBreak](#defaultAutoLineBreak) parameter for specific files.
_6p_auto_indent:
:	Define [defaultAutoIndent](#defaultAutoIndent) parameter for specific files.
_6p_smart_home:
:	Define [defaultSmartHome](#defaultSmartHome) parameter for specific files.
_6p_smart_paste:
:	Define [defaultSmartPaste](#defaultSmartPaste) parameter for specific files.
_6p_safe_indent:
:	Define [defaultSafeIndent](#defaultSafeIndent) parameter for specific files.

## maxRecentFiles
The maximum number of entries present in the recent files menu. Default to 10.

## tabsAtBottom
Whether or not tabs have to be displayed at the bottom of the window instead of at the top. Default to false.

