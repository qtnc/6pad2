# 6pad++ Python scripting API
Note: Import module `sixpad` in your scripts to get all functions and objects.

## Module sixpad

### Functions
- void setClipboardText(str newText), place a text in the system clipboard
- str getClipboardText(), return the contents of the system clipboard
- void setCurrentDirectory(str newDir), move the current directory
- str getCurrentDirectory(), return the current directory
- void include(str filename), run a file

### Members
- window, the window object (see below)

## The window object
### Methods
- void open(str filename), open a file in the editor
- void beep(int freq, int duration), produce a PC speaker beep
- void messageBeep(int id), produce a standard windows beep
- int messageBox(str text, str title, int flags), show a windows message box
- void alert(str text, str title), Show an alert dialog box with an OK button
- void warning(str text, str title), show a warning dialog box with an OK button
- int confirm(str text, str title), show a confirmation dialog box where the user can choose between yes and no; return 1 if the user clicked yes, 0 if he clicked no
- Menu createPopupMenu(), create a new popup menu and return it
- int addAccelerator(str key, callback function), install a callback to be called when the given shortcut key is pressed
- void addEvent(str eventName, callback function), install a callback to be called when the given event occurs; see the list of supported events further below
- void removeEvent(str eventName, callback function), remove an event previously registered

### Members
- Page curPage, the currently focused page in the editor
- [Page] pages, a list containing currently opened pages
- int pageCount, the number of pages currently open
- Menu menus, menu object representing the main menu bar

### Events
TBD

## Page class
TBD

## Menu class
TBD
