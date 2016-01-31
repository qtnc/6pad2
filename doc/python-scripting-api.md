%6pad++ Python scripting API
Note: Import module `sixpad` in your scripts to get all functions and objects.

# Module sixpad

## Functions

setClipboardText(newText) -> None:
:	Place a text in the system clipboard.
getClipboardText() -> str:
:	Return the contents of the system clipboard.
include(filename) -> None:
:	Run a file.
loadExtension(extensionName) -> None:
:	Load the specified extension; the name can be a python script to be included directly, a python module to be imported, or a C/C++ DLL extension.
loadTranslation (filename) -> None:
:	Load a translation .lng file.
msg(key) -> str:
:	Return the string associated to a given key in the translation file. Return the key itself if the string isn't found.
setConfig(key, value, multiple=False) -> None:
:	Set the value of a key in the configuration file. The multiple argument defines whether or not the key can be present multiple times. You can use keyword arguments 'key', 'value' and 'multiple'.
getConfig (key, defaultValue = '') -> str:
:	Return the value of a key from the configuration file. If the key has multiple values, the first value found is returned. If the key isn't found, defaultValue is returned.
getConfigAsList(key) -> [str]:
:	Return a list containing all the values associated with the given key in the configuration file.
say(text, interrupt=False) -> None:
:	IF a scren reader is active, speak the specified string. IF interrupt is True, the message is spoken immediately, possibly stopping any other speech; if interrupt is False (by default), the message is spoken as soon as possible once all other currently spoken messages are finished.
stopSpeech() -> None:
:	If a screen reader is currently active, it is requested to stop immediately speaking.
braille(text) -> None:
:	If a screen reader is currently active and if a braille display is connected, the given message is displayed on the braille display.

## Members
str locale:
:	The language of the application, e.g. `french`.
str appdir:
:	The application directory, e.g. `C:\6pad++\`.
st appname:
:	The name of the application executable, e.g. `6pad++`.
str appfullpath:
:	The full path of 6pad++ executable, e.g. `C:\6pad++\6pad++.exe`.
str configfile:
:	The path of the main 6pad++ configuration file, e.g. `C:\6pad++\6pad++.ini`.
[Window](#WindowObject) window:
:	The window object (see below)

# The window object {#WindowObject}
## Methods
open(filename) -> Page:
:	Open a file in the editor. If the file has been successfully opened in this instance, the page object is returned, otherwise None.
new (type = 'text') -> Page:
:	Open a new page of the type specified with an empty file. By default, only the type 'text' is supported, other plugins may support additional types.
beep(freq, duration) -> None:
:	Produce a PC speaker beep.
messageBeep(typeID) -> None:
:	Produce a standard windows beep.
messageBox(text, title, flags) -> int:
:	Show a windows message box.
alert(text, title = 'Info') -> None:
:	Show an alert dialog box with an OK button.
warning(text, title = 'Warning!') -> None:
:	Show a warning dialog box with an OK button.
confirm(text, title = 'Question') -> int:
:	Show a confirmation dialog box where the user can choose between yes and no; return 1 if the user clicked yes, 0 if he clicked no.
choice(prompt, title, listOfOptions, initialSelection=0) -> int:
:	Show a dialog box where the user can choose an option ammong a list. Returns the index of the chosen item, or -1 if the user cancelled the dialog box. You can use keyword arguments.
prompt(prompt, title, text='', list=[]) -> str:
:	Show a dialog box where the user can enter a single line of text. The list parameter allow to provide pre-entered suggestions. Returns the text entered by the user, or None if he cancelled the dialog box. You can use keyword arguments.
saveDialog(file='', title='', filters=[], initialFilter=0) -> multiple return types:
:	Show a save dialog box where the user can choose a file to save to. 
	If specified, the filters list must be a list of 2-tuples where the first item is the filter description e.g. "TExt files", and the second is the pattern e.g. "`*.txt`". 
	Returns a str indicating the path being selected by the user, or None if he cancelled the dialog box. 
	If a filters list is specified, returns a tuple where the first item is the selected file and the second is the index of the selected filter. 
	You can use keyword arguments.
openDialog(file='', title='', filters=[], initialFilter=0, multiple=False) -> multiple return types:
:	Show an open dialog box where the user can choose a file to open.
	If specified, the filters list must be a list of 2-tuples where the first item is the filter description e.g. "TExt files", and the second is the pattern e.g. "`*.txt`".
	Returns a str indicating the path being selected by the user, or None if he cancelled the dialog box. 
	If a filters list is specified, returns a tuple where the first item is the selected file and the second is the index of the selected filter.
	If multiple is set to True, the user can select more than one file to open and a list of str is returned instead of a single str.
	You can use keyword arguments.
chooseFolder(folder='', title='', root='', showFiles=False) -> str:
:	Show a dialog box where the user can choose a folder in a tree view. If showFiles is set to True, the user can also select files, otherwise he can only select folders. Returns the item selected by the user, or None if he cancelled the dialog box. You can use keyword arguments.
taskDialog(title=None, heading=None, text=None, details=None, footer=None, icon=None, buttons=None, radioButtons=None, checkbox=None, checked=False, collapseButtonText=None, expandButtonText=None, expanded=False, defaultButton=0, defaultRadioButton=0, expandInFooter=False, commandLinks=False, commandLinksNoIcon=False, progressBar=False, callback=None) -> (button,radio,checkbox)|TaskDialog:
:	Show a task dialog. Task dialogs offer a major evolution over old message boxes, allowing much more customization, flexibility and input controls. See the [dedicated chapter of this documentation](#taskDialog) for more information about the use of task dialogs and the different possible parameters.
focus () -> None:
:	Focus 6pad++ window.
showPopupMenu(listOfOptions) -> int:
:	Show a popup (context) menu with the given list of option items. Returns the index of the selected item in the list, or -1 if the user closed the menu without choosing an option.
addAccelerator(key, callbackFunction, specific = True) -> int:
:	Install a callback function to be called when the given shortcut key is pressed. If specific is set to True, the given shortcut key is specific to the current page, i.e. it is triggered only when pressed while being in the current page.
	ON success, return a non-zero identifier used to unregister this accelerator.
removeAccelerator(id) -> int:
:	Remove an accelerator previously installed. You must pass the event ID returned by addAccelerator.
addEvent(eventName, callbackFunction) -> int:
:	Install a callback function to be called when the given event occurs. See the [list of supported events for window](#windowEvents).
	ON success, return a non-zero identifier used to unregister this event.
removeEvent(eventName, eventID) -> int:
:	Remove an event previously registered. You must pass in the event ID previously returned by addEvent. Returns a non-zero value if the event has successfully been unregistered.
findAcceleratorByKey(key) -> int:
:	Look for an event associated to the given shortcut key. If found, its corresponding event ID is returned. IN case nothing is associated with the given shortcut key, 0 is returned.
findAcceleratorByID(id) -> str:
:	Look for a shortcut key associated with the given event ID. If the event ID doesn't exist, or if it isn't associated with a shortcut key, an empty string is returned.
setTimeout(callbackFunction, delay) -> int:
:	Schedule the given callback function to be called asynchronously after the specified delay. Returns a timer identifier, which can be passed to [clearTimeout](#clearTimeout) to cancel this timer.
setInterval(callbackFunction, interval) -> int:
:	Schedule the given callback function to be called asynchronously and repeatedly each specified interval until it is cancelled with a call to [clearInterval](#clearInterval). Returns the timer identifier, which must be given to `clearInterval` in order to stop this timer.
<span id="clearTimeout"></span>clearTimeout(timerId) -> None:
:	Cancel the specified timer.
<span id="clearInterval"></span>clearInterval(timerId) -> None:
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
The following events can be passed to addEvent/removeEvent for the window. In parenthesis are the arguments passed to the callback function.

activated ():
:	Called when the application window has been activated, i.e. made visible and got focus.
deactivated ():
:	Called when the application window has just been deactivated
close ():
:	Called when the application is about to be closed. By returning False, you can prevent the application from being closed.
	Note that this event is fired before page closes.
closed ():
:	Called when the application has just been closed.
pageOpened (newPage):
:	Called just after a new page has been opened
pageBeforeOpen (fileName) -> str|None:
:	Called before a new page is opened. The filename about to be opened is passed in the callback, and it is supposed to return the page type to create. By default, only the type 'text' is supported, but plugins may support additional types.
status (text) -> str|None:
:	Called when the status bar contents is about to be updated. The text which is about to be set is passed to the callback and you can return a new text string to overwrite what will be shown on the status bar.
	Note that this event occurs after the status event of a specific page.
title (text) -> str|None:
:	Called when the title of the application window is about to be changed. The title which is going to be set is passed in the callback, and you can return a new text string to overwrite what will be shown on the title bar of the application window.
fileDropped (file, x, y) -> bool:
:	Called when a file has been dragged from windows explorer and dropped onto the application window, or when a file from windows explorer is copied and pasted into the application  window. The callback receives the file name, and the mouse coordinates of the drop. In case of a clipboard operation, coordinates are (0;0).
	By returning True, the normal action is taken, i.e. open the dragged file in 6pad++. You can return False to prevent this action from happening.
	Note that this event is only triggered if no similar event is registered for the current page or if it returned True.

# Page class
## Methods
focus() -> None:
:	Focus the page.
close() -> None:
:	Close the page; a confirmation may be asked to the user.
addEvent(eventName, callbackFunction) -> int:
:	Install a callback to be called when the given event occurs at this page. See the [list of supported events for pages](#pageEvents).
	ON success, return a non-zero integer used to unregister this event.
removeEvent(eventName, eventId) -> int:
:	Remove an event previously registered. You must pass in the event ID previously returned by addEvent. Returns a non-zero value if the event has successfully been unregistered.
select(start, end) -> None:
:	Select a portion of text.
line(lineNumber) -> str:
:	Returns a line of text.
lineLength(lineNumber) -> int:
:	Returns the length of the given line.
lineOfOffset(position) -> int:
:	Return the line number corresponding to the character position given, or otherwise said, the line number where the given character position is found.
lineStartOffset(lineNumber) -> int:
:	Return the character position corresponding to the beginning of the given line number. First line is line 0.
lineEndOffset(lineNumber) -> int:
:	Return the character position corresponding to the end of the given line. First line is line 0.
lineSafeStartOffset(lineNumber) -> int:
:	Return the character position corresponding to the true beginning of the given line number, where the first non-space character is found. First line is line 0.
lineIndentLevel(lineNumber) -> int:
:	Return the indentation level of the given line, according to current indentation settings.
substring(start, end) -> str:
:	Gets a substring of the whole text currently being edited.
replace(start, end, newText) -> None:
:	Replace a range of characters by the given text.
delete(start, end) -> None:
:	Delete a range of characters.
insert(position, text) -> None:
:	Insert a string of text at the given position.
find(term, scase=False, regex=False, up=False, stealthty=False) -> int:
:	Make a search in the text, as if the user issued a search using the Find dialog. SEt scase to True for a sensible case search, regex to True for a regular expression search, and up to True for a search backward instead of forward. If stealthty is True, the find term won't be added in the combobox of previously searched terms in the Find dialog box. You can use keywords arguments. Returns 1 or 0 depending on if something has been found or not.
findNext() -> int:
:	Find the next occurence of the text previously searched for, as if the user pressed F3 or chose the Find next item in the Edit menu. Returns 1 if something has been found, 0 otherwise.
findPrevious() -> int:
:	Find the previous occurence of the text most recently searched for, as if the user pressed Shift+F3 or chose the Find previous item in the Edit menu. Returns 1 if something has been found, 0 otherwise.
searchReplace(search, replacement, scase=False, regex=False, stealthty=False) -> None:
:	Make a search/replace operation in the text, as if the user issued this command from the search/replace dialog box. SEt scase to True for a sensible case search, regex to True for a regular expression search/replace. If stealthty is True, terms won't be added in comboboxes of previously used terms in the dialog box. You can use keyword arguments.
save() -> None:
:	Save the file, as if file>save had been chosen by the user.
reload() -> None:
:	Reload the file, as if file>reload had been chosen by the user.
undo() -> None:
:	Undo the last operation, as if Edit>Undo had been chosen by the user.
redo() -> None:
:	Redo the last operation, as if Edit>Redo had been chosen by the user.
pushUndoState(UndoStateCompatibleObject) -> None:
:	Push an undoable operation on the top of the undo stack. The object passed must have the two methods undo(self,page) and redo(self,page).
doteditorconfig(key, defaultValue = '') -> str
:	Get the value of a configuration key that can be found in .editorconfig files for the current file. If the key is not found, defaultValue is returned. Note that .editorconfig configuration directives are loaded/updated when the file is loaded, reloaded or saved.

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
int tabWidth:
:	The width, in spaces, visually taken by a tab character, between 1 and 8.
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
:	The current line number where the caret is. If there is a selection, selectionEnd is taken as being the current position. Modifying this value is the same as doing `position = lineStartIndex(value)`, i.e. the cursor is placed at the beginning of the line.
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
:	Return a substring of text from the 5th to the 10th character, or the text between the beginning of the 5th line and the end of 10th line.

Replacing ranges, such as `object[4:9] = 'Hello'` also works, and overwrite the range specified.
However, note that *specifying a step different than 1*, (e.g. `object[10:20:3]` *isn't supported* and raises an error.

## Events
The following events can be passed to addEvent/removeEvent for a page. The first argument passed to the callback is always the page where the event occurred. Additional parameters passed are indicated in parenthesis.

activated ():
:	Called when the page has been activated, i.e. it ahs been made visible and focused because the user switched to this page.
deactivate () -> bool:
:	Called when a page is about to be deactivated, i.e. lose focus and become hidden because the user wants to switch to another page. By returning False, you can prevent the page from being deactivated.
deactivated ():
:	Called just after the page has been deactivated.
close () -> bool:
:	Called when the page is about to be closed (upon Ctrl+F4 or exiting the application). By returning False, you can prevent the page from being closed and the application from being exited. 
	Note that even if you accept the close by returning True or None, the user will still be asked whether he really wants to close this page. For security, there is no way to bypass this last confirmation.
closed ():
:	Called just after the page has been closed.
load (text) -> str|None:
:	Occurs when the text of the page is about to be loaded. The text which is going to appear in the edition field is passed to the callback. You can return a new string to overwrite the text being loaded.
beforeSave (fileName) -> str|None:
:	Occurs when the page is about to be saved. The file name is passed in the callback and you can return another file name in order to save to a different file. 
save (text) -> str|None:
:	Occurs when the page is about to be saved. The text that is about to be saved is passed in the callback. You can return a new text string to overwrite what is going to be saved.
status (text) -> str|None:
:	Occurs when the contents of the status bar is about to be updated. The text which is about to be displayed is passed to the callback, and you can return another text string to overwrite it.
enter (line) -> multiple return types:
:	Occurs when the user pressed the enter key in order to start a new line of text. The line of text where the user is before creating the new one is passed to the callback. You can return :
	- True or None to accept starting a new line
	- False to refuse to begin a new line; in this case, the caret isn't moved and no new line is started.
	- An int indicating the number of indentation levels to add or remove compared to the previous line
	- An str which will be automatically written at the beginning of the new line

keyPressed (ch) -> bool:
:	Occurs when a key press generating a character to write has been pressed. By returning False, you can prevent the character from being inserted into the edition area at the current caret position.
keyDown (vk) -> bool:
:	Occurs when the user press a key in the edition area. By returning False, you can prevent the default action of the key from happening.
keyUp (vk) -> bool:
:	Occurs when the user releases a key in the edition area. By returning False, you can prevent the default action of the key from happening.
contextmenu () -> bool:
:	Occurs when the user calls the context menu with application key and/or Shift+F10 and/or right mouse button. By returning False, you can prevent the default context menu from appearing.
fileDropped (file, x, y) -> bool:
:	Called when a file has been dragged from windows explorer and dropped onto the application window, or when a file from windows explorer is copied and pasted into the application  window. The callback receives the file name, and the mouse coordinates of the drop. In case of a clipboard operation, coordinates are (0;0).
	By returning True, the normal action is taken, i.e. open the dragged file in 6pad++. You can return False to prevent this action from happening.

# Menu class {#menuobj}
## Methods
add(label='', action=None, index=-1, accelerator='', name='', submenu=False, separator=False, specific=False) -> Menu:
:	Add a new item to the menu and returns it. Specify a serie of keyword arguments to define what kind of menu item to add:
	- label=text of the item
	- action=the callback function to call when the item is chosen by the user
	- index=the position of the new item
	- accelerator=a string specifying a shortcut key such a 'Ctrl+E'
	- name=The name of the submenu allowing it to be referenced in python code
	- submenu=True to create a submenu rahter than a menu item
	- specific=True to create a page-specific menu element, i.e. it appears only when the current page is active
	- separator=True to create an item separator

remove(name=None, index=-1) -> None:
:	Remove this item or a subitem; without any argument, remove this item from the menu; specifiy wheither the keyword argument name or index to remove a subitem by its name or index.

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

IN any case, None is returned if the item is not found.
In case this menu object represents a single menu item, you can also use the regular call operator like a function to trigger the action normally executed by the item.
I.e. `item()`. If the item represents a submenu, calling it like a function raises an error.

- *Warning: slicing, e.g. menu[1:3] doesn't work* and raise an error.

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
callable action:
:	The action function that has to be called when the item is clicked. This is None for submenus or built-in menu items (i.e. items that are natively managed).

# Task dialogs and TaskDialog class {#taskDialog}
Task dialogs offer a major evolution over old message boxes, allowing much more customization, flexibility and input controls. They are described [on the MSDN](https://msdn.microsoft.com/en-us/library/windows/desktop/bb787471(v=vs.85).aspx).
To show a task dialog, first call the `taskDialog` method of the window object. Its long list of parameters is explained below.

taskDialog(title=None, heading=None, text=None, details=None, footer=None, icon=None, buttons=None, radioButtons=None, checkbox=None, checked=False, collapseButtonText=None, expandButtonText=None, expanded=False, defaultButton=0, defaultRadioButton=0, expandInFooter=False, commandLinks=False, commandLinksNoIcon=False, progressBar=False, callback=None) -> (button,radio,checkbox)|TaskDialog

title:
:	The title of the dialog box, as shown in the title bar
heading:
:	the main text of the dialog box, shortly explaining its purpose. referred as main instructions in the MSDN doc. Often telling a simple fact on which we would like the user to decide on what action to take. Example: : "foo.txt has changed since the last save". The heading isn't shown at all if it is unspecified or blank.
text:
:	additional text shown below the main text heading, explaing a bit more the main purpose of the dialog box. Referred as content in the MSDN doc. Often asking a question to the user, such as "Do you want to save changes to foo.txt?" or to give a little more important precisions than the heading. The content isn't shown at all if it is unspecified or blank.
details:
:	One more additional detail text that the user can make visible by pressing on a "More details" button. Usually, this text isn't displayed immediately when the dialog is open, unless by pressing on the expand button. Refered as expanded information in the MSDN. For example, you can put detailed error report in that place. If unspecified or blank, neither the details area nor the expand/collapse details button are displayed.
footer:
:	A text that appears underneath the buttons, in a smaller font. If None or blank, no space is reserved to display this text.
icon:
:	A string indicating which icon must illustrate the dialog box. This parameter can be one of the strings "none", "info", "warning", "error" or "shield", by default "none", meaning no icon has to be displayed at all. This parameter also controls the sound emited when the dialog box is opened, i.e. asterisk, exclamation and critical stop for resp. "info", "warning" and "error". NO sound is produced when using "none" or "shield" icons.
buttons:
:	A list or tuple of one or more strings indicating the different buttons that will be present on the dialog box. If no button is specified, because the list/tuple is empty or if None is passed, the dialog box shows up with a single OK button.
defaultButton:
:	Index of the button selected by default when the dialog box initially pops-up. If unspecified, 0 is used, i.e. the first button is selected by default.
radioButtons:
:	A list or tuple of two or more strings specifying the different radio buttons to show in the dialog box. A task dialog can show a group of radio buttons, which the user can select an option from (as usual with radio buttons, a single option at a time). If this parameter is None or an empty list/tuple, no space is taken to display any radio buttons.
defaultRadioButton:
:	The index of the radio button initially selected when the dialog box pops-up. BY default, 0, i.e. the first radio button is preselected.
checkbox:
:	Text label of a verification checkbox. A task dialog can also display a single checkbox, often used  to give options like "Don't show this dialog again" or "Apply my choice to all following elements". IF this isn't specified or blank, no space is taken to display any checkbox.
checked:
:	Indicate whether the checkbox is initially checked or not when the dialog pops-up. BY default False (unchecked). No effect if no checkbox text has been defined.
expandButtonText:
:	Text of the expand button used to show the details. If None or blank, a default label such as "More details" will be used. No effect if no details text is defined, since expand/collapse button isn't displayed in that case.
collapseButtonText:
:	Text of the expand button to hide the details. If None or blank, a default label such as "More details" will be used. No effect if no details text is defined, since expand/collapse button isn't displayed in that case.
expanded
:	Indicates if detail text is initially displayed (expanded) when the dialog pops-up. By default False (collapsed / details not shown). No effect if no details text is defined.
expandInFooter:
:	If this parameter is true, details are placed below the buttons when they are expanded, instead of above. By default False. No effect if no details text is defined.
commandLinks:
:	If this parameter is True, buttons are replaced by command links. They work the same, they just have the appearance of web links instead of classic buttons.
commandLinksNoIcon:
:	BY default when buttons are replaced by command links, there are small icons on their left. If this parameter is True, these icon are removed. By default False. This parameter has no effect if commandLinks=False.
progressBar:
:	When set to True, a progress bar is shown in the dialog box and a TaskDialog object is returned instead of a tuple of results. This allows to turn a task dialog into a monitor progress dialog.
	If this parameter is specified, the taskDialog method has to be called from a working thread, and not the main UI thread. Otherwise, the sript is blocked until the method returns and you have no way to update the progress bar as the task is making progress.
callback:
:	Define a callback to be called whenever an event occurs in the dialog box (see further down).

If the progressBar is unspecified or False, the *taskDialog* method returns a 3-tuple of integers indicating, in order:
- Which button has been pressed, as an index in the buttons list/tuple passed, or -1 if the user closed the dialog box by pressing escape or Alt+F4.
- Which radio button has been selected, as an index in the radio buttons list/tuple passed, or -1 if there was no radio buttons in the dialog box.
- 1 or 0 depending on the state checked or unchecked of the verification checkbox, or -1 if there was no checkbox in the dialog.

If progressBar was set to True however, the *taskDialog* method returns a TaskDialog object, which you can manipulate as long as the dialog remains open, typically until the task in progress finished or when the user cancelled it.
This object has the properties and method described below.

## Methos
close() -> None:
:	Close the dialog box by simulating a press on the *Cancel* button or the escape key. After this call, the dialog box is closed, and buttonClicked=-1.
enableButton(buttonIndex, bEnable) -> None:
:	Enable or disable the specified button in the dialog box.
enableRadioButton(radioButtonIndex, bEnable) -> None:
:	Enable or disable the specified radio button in the dialog box.

## Members
int closed (read only):
:	Indicate whether the dialog box is still open, or already closed. Any further modification of the other members after the dialog is closed has no effect.
float value:
:	The value of the progress bar, between 0 (nothing is done) to 1 (fully complete)
str title:
:	The title of the dialog box, as displayed on the title bar
str heading:
:	The dialog's heading text
str text:
:	The dialog's content text
str details:
:	The dialog's details text, shown when clicking on the "More details" button.
str footer:
:	The dialog's footer text
int buttonClicked (read only):
:	Once the dialog is closed, indicates the index of the button clicked by the user in the buttons list/tuple, or -1 if escape or Alt+F4 was used to close the dialog box. Since the close method simulates a click, buttonClicked always equals -1 after an explicit call to close.
int radioChecked:
:	Indicates the radio button currently selected by the user as an index in the radio buttons list/tuple, or -1 if the dialog doesn't contain any radio button. You can modify this property to programmatically select another radio button.
int checkboxChecked:
:	Indicate if the verification checkbox is currently checked or not. You can change the state of the checkbox by modifying this property.


## The task dialog callback
The task dialog callback is called when some particular events occurrs within a task dialog. The callback to be called is specified with the parameter *callback* of the *taskDialog* method.
The callback obtain two arguments: an object TaskDialog representing the task dialog, and an int indicating which event occurred. Here are the different values you can receive and the events to which they correspond:

1000...1999:
:	One of the button has been clicked. 1000 corresponds to button 0 in the list/tuple, 1001 to button 1, and so on.
	You can return True to close the task dialog, or False to keep it open. If you don't return anything, any click on a button closes the dialog.
2000...2999:
:	One of the radio buttons has been selected. 2000 corresponds to the first radio button in the list/tuple, 2001 to the second, and so on.
3000:
:	The verification checkbox has been clicked and it is no unchecked
3001:
:	The verification checkbox has been clicked and it is no checked
3002:
:	The expand/collapse details button has been clicked and details are now hidden/collapsed
3003:
:	The expand/collapse details button has been clicked and details are now shown/expanded.
