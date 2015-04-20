%6pad++ Python scripting API
Note: Import module `sixpad` in your scripts to get all functions and objects.

# Module sixpad

## Functions

void setClipboardText(str newText):
:	Place a text in the system clipboard.
str getClipboardText():
:	Return the contents of the system clipboard.
void setCurrentDirectory(str newDir):
:	Move the current directory.
str getCurrentDirectory():
:	Return the current directory.
void include(str filename):
:	Run a file.

## Members
window:
:	The window object (see below)

# The window object
## Methods
void open(str filename):
:	Open a file in the editor.
void beep(int freq, int duration):
:	Produce a PC speaker beep.
void messageBeep(int id):
:	Produce a standard windows beep.
int messageBox(str text, str title, int flags):
:	Show a windows message box.
void alert(str text, str title):
:	Show an alert dialog box with an OK button.
void warning(str text, str title):
:	Show a warning dialog box with an OK button.
int confirm(str text, str title):
:	Show a confirmation dialog box where the user can choose between yes and no; return 1 if the user clicked yes, 0 if he clicked no.
<span id="wcpm1"></span>Menu createPopupMenu(): 
:	Create a new popup menu and return it.
int addAccelerator(str key, callback function):
:	Install a callback to be called when the given shortcut key is pressed.
void addEvent(str eventName, callback function):
:	Install a callback to be called when the given event occurs. See the [list of supported events for window](#windowEvents).
void removeEvent(str eventName, callback function):
:	Remove an event previously registered.

## Members
Page curPage:
:	The currently focused page in the editor.
[Page] pages:
:	A list containing currently opened pages.
int pageCount:
:	The number of pages currently open in the editor.
Menu menus:
:	Menu object representing the main menu bar.

## Events {#windowEvents}
The following events can be passed to addEvent/removeEvent for the window. In parenthesis are the arguments passed to the callback .

activated ():
:	Called when the application window has been activated, i.e. made visible and got focus.
deactivated ():
:	Called when the application window has just been deactivated
opened ():
:	Called just after the application window has been opened
close ():
:	Called when the application is about to be closed. By returning False, you can prevent the application from being closed.
	Note that this event is fired before page closes.
closed ():
:	Called when the application has just been closed.

# Page class
## Methods
void focus():
:	Focus the page.
void close():
:	Close the page; a confirmation may be asked to the user.
void addEvent(str eventName, callback function):
:	Install a callback to be called when the given event occurs at this page. See the [list of supported events for pages](#pageEvents).
void removeEvent(str eventName, callback function):
:	Remove an event previously registered.
void select(int start, int end):
:	Select a portion of text.
str line(int lineNumber):
:	Returns a line of text.
int lineLength(int lineNumber):
:	Returns the length of the given line.
lineOfOffset(int position):
:	Return the line number corresponding to the character position given, or otherwise said, the line number where the given character position is found.
lineStartOffset(int lineNumber):
:	Return the character position corresponding to the beginning of the given line number. First line is line 0.
lineEndOffset(int lineNumber):
:	Return the character position corresponding to the end of the given line. First line is line 0.
substring(int start, int end):
:	Gets a substring of the whole text currently present in being edited.
replace(int start, int end, str text):
:	Replace a range of characters by the given text.
delete(int start, int end):
:	Delete a range of characters.
insert(int position, str text):
:	Insert a string of text at the given position.
void undo():
:	Undo the last operation.
void redo():
:	Redo the last operation.
void pushUndoState(object):
:	Push an undoable operation on the top of the undo stack. The object passed must have the two methods undo(self,page) and redo(self,page).

## Members
int closed (read only):
:	Indicates if the page has been closed by the user.
int modified (read only):
:	Indicates if the contents of the page has been modified since the last load/save.
str name:
:	The name of the page as shown on the tablist and on the window title.
str file:
:	The file to which data are to be loaded/saved.
int lineEnding:
:	The current line ending convention to use when saving the file:
	- 0=CRLF/windows
	- 1=LF/UNIX
	- 2=CR/Mac

int encoding:
:	The encoding to use when saving the file: 
	- 0=ANSI
	- 65001=UTF-8
	- 65002=UTF-8+BOM
	- 1200=UTF-16Le
	- 1201=UTF-16Be
	- 1202=UTF-16Le+BOM
	- 1203=UTF-16Be+BOM
	- 28605=ISO-8859-15
	- see <https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756(v=vs.85).aspx> for other values.

int indentation:
:	The indentation convention to use while editing the file: 0=tabs, 1-8=1 to 8 spaces
int autoLineBreak:
:	Whether or not lines are broken automatically when displaying the text in the edition field.
int selectionStart:
:	The anchor point of the current selection.
int selectionEnd:
:	The end point of the current selection.
str selectedText:
:	The text currently selected; if there is no selection, this string is empty.
str text:
:	The current text being edited.
int textLength (read only):
:	The length of the whole text currently present in the editor; equivalent to `len(text)`.
int lineCount (read only):
:	The number of lines composing the text being edited.

## Events
The following events can be passed to addEvent/removeEvent for a page. The first argument passed to the callback is always the page where the event occurred. Additional parameters passed are indicated in parenthesis.

activated ():
:	Called when the page has been activated, i.e. it ahs been made visible and focused because the user switched to this page.
deactivate ():
:	Called when a page is about to be deactivated, i.e. lose focus and become hidden because the user wants to switch to another page. By returning False, you can prevent the page from being deactivated.
deactivated ():
:	Called just after the page has been deactivated.
close ():
:	Called when the page is about to be closed (upon Ctrl+F4 or exiting the application). By returning False, you can prevent the page from being closed and the application from being exited. 
	Note that even if you accept the close by returning True or None, the user will still be asked whether he really wants to close this page. For security, there is no way to bypass this last confirmation.
closed ():
:	Called just after the page has been closed.
load (str text):
:	Occurs when the text of the page is about to be loaded. The text which is going to appear in the edition field is passed to the callback. You can return a new string to overwrite the text being loaded.
save (str text):
:	Occurs when the page is about to be saved. The text that is about to be saved is passed in the callback. You can return a new text string to overwrite what is going to be saved.
status (str text):
:	Occurs when the contents of the status bar is about to be updated. The text which is about to be displayed is passed to the callback, and you can return another text string to overwrite it.
enter (str line):
:	Occurs when the user pressed the enter key in order to start a new line of text. The line of text where the user is before creating the new one is passed to the callback. You can return :
	- True or None to accept starting a new line
	- False to refuse to begin a new line; in this case, the caret isn't moved and no new line is started.
	- An int indicating the number of indentation levels to add or remove compared to the previous line
	- An str which will be automatically written at the beginning of the new line

keyPressed (int ch):
:	Occurs when a key press generating a character to write has been pressed. By returning False, you can prevent the character from being inserted into the edition area at the current caret position.
keyDown (int vk):
:	Occurs when the user press a key in the edition area. By returning False, you can prevent the default action of the key from happening.
keyUp (int vk):
:	Occurs when the user releases a key in the edition area. By returning False, you can prevent the default action of the key from happening.
contextmenu ():
:	Occurs when the user calls the context menu with application key and/or Shift+F10 and/or right mouse button. By returning False, you can prevent the default context menu from appearing.

# Menu class
## Methods
Menu add(...):
:	Add a new item to the menu and returns it. Specify a serie of keyword arguments to define what kind of menu item to add:
	- label=text of the item
	- action=the callback function to call when the item is chosen by the user
	- index=the position of the new item
	- accelerator=a string specifying a shortcut key such a 'Ctrl+E'
	- name=The name of the submenu allowing it to be referenced in python code
	- submenu=True to create a submenu rahter than a menu item
	- separator=True to create an item separator

void remove(...):
:	Remove this item or a subitem; without any argument, remove this item from the menu; specifiy wheither the keyword argument name or index to remove a subitem by its name or index.
int show():
:	Make this menu appear to the user; this can work only if this menu is a popup menu (see [window.createPopupMenu](#wcpm1)). Return the command identifier of the item selected by the user, or 0 in case he cancelled the menu or if it failed to display for some reason.

## Mapping/indexing {#mappingindexing}
You can use the indexing operator `[...]`or the direct attribute/member operator `.` to obtain a sub menu item of this menu, by its name or its index.
- menu[0], return the first item of this menu
- menu[-1], return the last item of this menu
- menu['itemName'], return the sub menu item having the name 'itemName'
- menu.itemName equivalent to menu['itemName']
- None is returned if the item is not found
- *Warning: menu[1:3] doesn't work*

## Members
int submenu (read only):
:	Whether this object represents a menu or a single menu item.
int length (read only):
:	The number of sub menu items contained in this menu, or -1 if this object represents just a single menu item.
Menu parent (read only):
:	The parent menu of this menu item. This is never None except for the object representing the main menu bar.
str name:
:	The name of the item allowing to access it in python code (see [mapping/indexing](#mappingindexing)).
str label:
:	The text label of the item.
str accelerator:
:	A string representing the key shortcut associated with the item (e.g. 'Ctrl+E'). Empty if no accelerator is associated with this item.
int id (read only):
:	The command identifier of this item.
int enabled:
:	The enabled/disabled state of the item.
int checked:
:	The checked/unchecked state of the item.
int radio:
:	Whether or not the item is a radio button menu item.
