%6pad++ Python scripting API
Note: Import module `sixpad` in your scripts to get all functions and objects.

# Module sixpad

## Functions

void setClipboardText(str newText):
:	Place a text in the system clipboard.
str getClipboardText():
:	Return the contents of the system clipboard.
void include(str filename):
:	Run a file.
void loadExtension(str extensionName)
:	Load the specified extension; the name can be a python script to be included directly, a python module to be imported, or a C/C++ DLL extension.
void loadTranslation (str filename):
:	Load a translation .lng file.
str msg(str key):
:	Return the string associated to a given key in the translation file. Return the key itself if the string isn't found.
setConfig(str key, str value, [Bool multiple=False]):
:	Set the value of a key in the configuration file. The multiple argument defines whether or not the key can be present multiple times. You can use keyword arguments 'key', 'value' and 'multiple'.
str getConfig (str key, str defaultValue):
:	Return the value of a key from the configuration file. If the key has multiple values, the first value found is returned. If the key isn't found, defaultValue is returned.
[str] getConfigAsList(str key):
:	Return a list containing all the values associated with the given key in the configuration file.
say(str text, [bool interrupt=False]):
:	IF a scren reader is active, speak the specified string. IF interrupt is True, the message is spoken immediately, possibly stopping any other speech; if interrupt is False (by default), the message is spoken as soon as possible once all other currently spoken messages are finished.
stopSpeech():
:	If a screen reader is currently active, it is requested to stop immediately speaking.
braille(str text):
:	If a screen reader is currently active and if a braille display is connected, the given message is displayed on the braille display.

## Members
locale:
:	The language of the application, e.g. `french`.
appdir:
:	The application directory, e.g. `C:\6pad++\`.
appname:
:	The name of the application executable, e.g. `6pad++`.
appfullpath:
:	The full path of 6pad++ executable, e.g. `C:\6pad++\6pad++.exe`.
configfile:
:	The path of the main 6pad++ configuration file, e.g. `C:\6pad++\6pad++.ini`.
window:
:	The window object (see below)

# The window object
## Methods
Page open(str filename):
:	Open a file in the editor. If the file has been successfully opened in this instance, the page object is returned, otherwise None.
Page new (str type):
:	Open a new page of the type specified with an empty file. By default, only the type 'text' is supported, other plugins may support additional types.
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
int choice(str prompt, str title, list options, [int initialSelection=0]):
:	Show a dialog box where the user can choose an option ammong a list. Returns the index of the chosen item, or -1 if the user cancelled the dialog box.
<span id="wcpm1"></span>Menu createPopupMenu(): 
:	Create a new popup menu and return it.
int addAccelerator(str key, callback function):
:	Install a callback to be called when the given shortcut key is pressed.
	ON success, return a non-zero identifier used to unregister this accelerator.
int removeAccelerator(int id):
:	Remove an accelerator previously installed. You must pass the event ID returned by addAccelerator.
int addEvent(str eventName, callback function):
:	Install a callback to be called when the given event occurs. See the [list of supported events for window](#windowEvents).
	ON success, return a non-zero identifier used to unregister this event.
int removeEvent(str eventName, int eventId):
:	Remove an event previously registered. You must pass in the event ID previously returned by addEvent. Returns a non-zero value if the event has successfully been unregistered.
int findAcceleratorByKey(str key):
:	Look for an event associated to the given shortcut key. If found, its corresponding event ID is returned. IN case nothing is associated with the given shortcut key, 0 is returned.
str findAcceleratorByID(int id):
:	Look for a shortcut key associated with the given event ID. If the event ID doesn't exist, or if it isn't associated with a shortcut key, an empty string is returned.
int setTimeout(callback function, int delay):
:	Schedule the given callback function to be called asynchronously after the specified delay. Returns a timer identifier, which can be passed to [clearTimeout](#clearTimeout) to cancel this timer.
int setInterval(callback function, int interval):
:	Schedule the given callback function to be called asynchronously and repeatedly each specified interval until it is cancelled with a call to [clearInterval](#clearInterval). Returns the timer identifier, which must be given to `clearInterval` in order to stop this timer.
<span id="clearTimeout"></span>clearTimeout(int timerId)
:	Cancel the specified timer.
<span id="clearInterval"></span>clearInterval(int timerId)
:	Cancel the specified timer.

## Members
Page curPage:
:	The currently focused page in the editor.
[Page] pages:
:	A list containing currently opened pages.
int pageCount:
:	The number of pages currently open in the editor.
Menu menus:
:	Menu object representing the main menu bar. See [Menu object](#menuobj).
str status:
:	The text of the status bar.
str title:
:	The window title.

## Events {#windowEvents}
The following events can be passed to addEvent/removeEvent for the window. In parenthesis are the arguments passed to the callback .

activated ():
:	Called when the application window has been activated, i.e. made visible and got focus.
deactivated ():
:	Called when the application window has just been deactivated
close ():
:	Called when the application is about to be closed. By returning False, you can prevent the application from being closed.
	Note that this event is fired before page closes.
closed ():
:	Called when the application has just been closed.
pageOpened (Page newPage):
:	Called just after a new page has been opened
pageBeforeOpen (str fileName):
:	Called before a new page is opened. The filename about to be opened is passed in the callback, and it is supposed to return the page type to create. By default, only the type 'text' is supported, but plugins may support additional types.
status (str text):
:	Called when the status bar contents is about to be updated. The text which is about to be set is passed to the callback and you can return a new text string to overwrite what will be shown on the status bar.
	Note that this event occurs after the status event of a specific page.
title (str text):
:	Called when the title of the application window is about to be changed. The title which is going to be set is passed in the callback, and you can return a new text string to overwrite what will be shown on the title bar of the application window.
fileDropped (str file, int x, int y):
:	Called when a file has been dragged from windows explorer and dropped onto the application window, or when a file from windows explorer is copied and pasted into the application  window. The callback receives the file name, and the mouse coordinates of the drop. In case of a clipboard operation, coordinates are (0;0).
	By returning True, the normal action is taken, i.e. open the dragged file in 6pad++. You can return False to prevent this action from happening.
	Note that this event is only triggered if no similar event is registered for the current page or if it returned True.

# Page class
## Methods
void focus():
:	Focus the page.
void close():
:	Close the page; a confirmation may be asked to the user.
int addEvent(str eventName, callback function):
:	Install a callback to be called when the given event occurs at this page. See the [list of supported events for pages](#pageEvents).
	ON success, return a non-zero integer used to unregister this event.
int removeEvent(str eventName, int eventId):
:	Remove an event previously registered. You must pass in the event ID previously returned by addEvent. Returns a non-zero value if the event has successfully been unregistered.
void select(int start, int end):
:	Select a portion of text.
str line(int lineNumber):
:	Returns a line of text.
int lineLength(int lineNumber):
:	Returns the length of the given line.
int lineOfOffset(int position):
:	Return the line number corresponding to the character position given, or otherwise said, the line number where the given character position is found.
int lineStartOffset(int lineNumber):
:	Return the character position corresponding to the beginning of the given line number. First line is line 0.
int lineEndOffset(int lineNumber):
:	Return the character position corresponding to the end of the given line. First line is line 0.
int lineSafeStartOffset(ine lineNumber):
:	Return the character position corresponding to the true beginning of the given line number, where the first non-space character is found. First line is line 0.
int lineIndentLevel(int lineNumber):
:	Return the indentation level of the given line.
str substring(int start, int end):
:	Gets a substring of the whole text currently present in being edited.
void replace(int start, int end, str text):
:	Replace a range of characters by the given text.
void delete(int start, int end):
:	Delete a range of characters.
void insert(int position, str text):
:	Insert a string of text at the given position.
void find(str text, bool scase, bool regex, bool up):
:	Make a search in the text, as if the user issued a search using the Find dialog. SEt scase to True for a sensible case search, regex to True for a regular expression search, and up to True for a search backward instead of forward.
void findNext():
:	Find the next occurence of the text previously searched for, as if the user pressed F3 or chose the Find next item in the Edit menu.
void findPrevious():
:	Find the previous occurence of the text most recently searched for, as if the user pressed Shift+F3 or chose the Find previous item in the Edit menu.
void save():
:	Save the file, as if file>save had been chosen.
void reload():
:	Reload the file, as if file>reload had been chosen.
void undo():
:	Undo the last operation.
void redo():
:	Redo the last operation.
void pushUndoState(object):
:	Push an undoable operation on the top of the undo stack. The object passed must have the two methods undo(self,page) and redo(self,page).

## Members
int closed (read only):
:	Indicates if the page has been closed by the user.
int modified:
:	Indicates if the contents of the page has been modified since the last load/save.
str name:
:	The name of the page as shown on the tablist and on the window title.
str file:
:	The file to which data are to be loaded/saved.
int lineEnding:
:	The current line ending convention to use when saving the file. See [line endings](configuration.html#le) for a list of possible values.
int encoding:
:	The encoding to use when saving the file. See [Character encodings](configuration.html#ce) for a list of possible values.
int indentation:
:	The indentation convention to use while editing the file. See [indentation modes](configuration.html#indent) for a list of possible values.
int autoLineBreak:
:	Whether or not lines are broken automatically when displaying the text in the edition field.
int selectionStart:
:	The anchor point of the current selection.
int selectionEnd:
:	The end point of the current selection.
int position
:	Same as selectionEnd. Modifying this attribute is the same as doint `select(value, value)`, i.e. the selection is collapsed at given position.
int curColumn (read only):
:	The current column where the caret is. If there is a selection, selectionEnd is taken as being the current position.
int curLine:
:	The current line number where the caret is. If there is a selection, selectionEnd is taken as being the current position. Modifying this value is the same as doing `position = lineStartIndex(value)`.
str curLineText:
:	The text of the line where the caret currently is. If there is a selection, selectionEnd is taken as being the current position.
str selectedText:
:	The text currently selected; if there is no selection, this string is empty.
str text:
:	The current text being edited.
int textLength (read only):
:	The length of the whole text currently present in the editor; equivalent to `len(text)`.
int lineCount (read only):
:	The number of lines composing the text being edited.
str indentString:
:	A string representing a level of indentation, i.e. a tab or a couple of spaces.
<span id="rangesinlines"></span>int rangesInLines:
:	Whether indexing and slicing indices are counted in lines. If this attribute is False, indices are counted in characters. By default, this attribute is False. See [Indexing and slicing](#pageindexing) for more details.

## Indexing and slicing {#pageindexing}
On a page object, you can use the indexing and slicing operator to obtain a substring or lines of text.
Depending on the value of the [rangesInLines](#rangesinlines) attribute, the indices you give are expressed in characters or in lines.

object[0]:
:	Return the first character or the first line of text.
object[4:9]:
:	Return a substring of text from the 3rd to the 8th character, or the text between the beginning of the 3rd line and the end of 7th line.

Replacing ranges, such as `object[4:9] = 'Hello'` also works, and overwrite the range specified.
However, note that *specifying a step different than 1*, (e.g. `object[10:20:3]` *isn't supported* and raises an error.

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
beforeSave (str fileName):
:	Occurs when the page is about to be saved. The file name is passed in the callback and you can return another file name in order to save to a different file, 
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
fileDropped (str file, int x, int y):
:	Called when a file has been dragged from windows explorer and dropped onto the application window, or when a file from windows explorer is copied and pasted into the application  window. The callback receives the file name, and the mouse coordinates of the drop. In case of a clipboard operation, coordinates are (0;0).
	By returning True, the normal action is taken, i.e. open the dragged file in 6pad++. You can return False to prevent this action from happening.

# Menu class {#menuobj}
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

## Mapping, indexing and calling {#mappingindexing}
You can use the indexing operator `[...]`or the direct attribute/member operator `.` to obtain a sub menu item of this menu, by its name or its index.

menu[0]:
:	Return the first item of this menu.
menu[-1]:
:	Return the last item of this menu.
menu['itemName']:
:	Return the sub menu item having the name 'itemName'.
menu.itemName:
:	Equivalent to menu['itemName'].

IN any case, None is returned if the item is not found
- *Warning: slicing, e.g. menu[1:3] doesn't work*

In case this menu object represents a single menu item, you can also use the regular call operator like a function to trigger the action normally executed by the item.
I.e. `item()`. If the item represents a submenu, calling it like a function raises an error.

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
