%6pad++ Additional dialogs plugin
Note: Import module `qc6paddlgs` in your scripts to get all functions and objects.

# TreeViewDialog class {#tvddef1}
The TreeViewDialog allows you to create modal and modless dialogs containing a tree view as their main control.
The tree view can be used to display hierarchical data such as files/directories, XML-like structures, etc.
To open a new tree view dialog and create an object, you must use the `open` static method described below.

## Static methods
open(title='', hint='', modal=False, multiple=False, editable=False, okButtonText='', cancelButtonText='', callback=None) -> multiple possible return types:
:	Create and open a new tree view dialog. Returns the value(s) of the item(s) selected by the user if modal=True, or a TreeViewDialog object if modal=False. Use keywords to set the different options of the dialog box:
	- title = The title text of the dialog box, shown on the dialog box title bar
	- hint = a little text hint, placed above the tree view
	- modal = whether or not the dialog has to be modal. If modal is True, the function returns the selected item value chosen by the user, or a list of item values if multiple=True. If modal=False, then a TreeViewDialog object is returned, allowing further updates of the tree view's contents as long as the dialog remains open, including while being in background (not focused).
	- multiple = whether the user can choose one or several items at the same time. If multiple is True, a checkbox is placed on the left of each item, allowing the user to independently selectt items. When the user checks or unchecks an item that has children, all children are automatically checked or unchecked recursively. An item become partially checked if one or more, but not all of its children are checked.
	- editable = whether or not the user is able to edit the text of the items, by pressing F2 or double-clicking slowly on an item.
	- okButtonText = the text of the OK button. If None, then the button isn't shown. If the empty string, then a default localized text is set.
	- cancelButtonText = the text of the Cancel button. If None, then the button isn't shown. If the empty string, then a default localized text is set.
	- callback = a function that is going to be called when the dialog box is initialized. You can use this callback to populate the tree. It receives the TreeViewDialog object just created as its only parameter.

## Methods
addEvent(eventName, callbackFunction) -> int:
:	Install a callback function to be called when the given event occurs. See the [list of supported events for TreeViewDialogw](#tvdEvents).
	ON success, return a non-zero identifier used to unregister this event.
removeEvent(eventName, eventID) -> int:
:	Remove an event previously registered. You must pass in the event ID previously returned by addEvent. Returns a non-zero value if the event has successfully been unregistered.
focus() -> None:
:	If the dialog is non-modal, it is made visible and focused if it isn't already the case.
close() -> None:
:	If the dialog is non-modal, close it.

## Members
closed
:	Whether or not the dialog is closed.
title:
:	The title text of the dialog box
text:
:	The hint text placed above the tree view.
root:
:	The [TreeViewItem](#tvddef) object representing the root element of the tree.
selectedItem (read only):
:	The [TreeViewItem](#tvddef) currently selected by the user, i.e. the one that has focus.
selectedValue (read only):
:	The value associated with the currently selected item. This is a shortcut for `selectedItem.value`.
selectedItems (read only):
:	In case the tree view allows multiple selection with checkboxes, give a list of currently checked items.
selectedValues (Read only):
:	In case the tree view allows multiple selection with checkboxes, gives a lisf containing the values associated with checked items. Can be a shortcut for `[x.value for x in selectedItems]`.

## Events {#tvdEvents}
The following events can be passed to addEvent/removeEvent for the TreeViewDialog. In parenthesis are the arguments passed to the callback function.

action (dialog, item):
:	Called when the user pressed enter or double-clicked an item.
contextMenu (dialog, item):
:	Called when the user requests the context menu for an item, i.e. when he pressed the application key or Shift+F10, or right-clicked an item
select (dialog, item):
:	Called each time another item is selected (focused).
check (dialog, item):
:	For a multiple selection tree view with checkboxes only, called each time the user checks or unchecks an item by clicking on the checkbox or pressing space.
expand (dialog, item):
:	Called when an item is about to be expanded. You can return True to accept the expansion, or False to refuse it (the item will stay collapsed in this case).
edit (dialog, item):
:	Called when the user wants to edit the text content of an item, before entering in edition mode. You can return True to accept the edition, or False to refuse.
edited (dialog, item, text):
:	Called when the user finished editing an item. You can return True to accept the change, False to refuse it, or an str to overwrite the label which is about to be changed.
close (dialog):
:	Called when the user is about to close the dialog box. You can return True to accept closing it, or False to keep it open.
focus (dialog):
:	Called when the tree view is focused.
blur (dialog):
:	Called when the tree view loses focus.

# TreeViewItem class {#tvddef}
A TreeViewItem represents an item within a tree view managed by a [TreeViewDialog](#tvddef1).

## Methods
appendChild(text, value, checked=False, partiallyChecked=False, selected=False, expanded=False):
:	Append  a new item at the end of this item's children list. Text and value are mandatory. You can use keyword arguments to set other flags.
insertBefore(reference, text, value, checked=False, partiallyChecked=False, selected=False, expanded=False):
:	Insert a new item in this item's children list, before the element specified as reference. Text and value are mandatory. You can use keyword arguments to set other flags.
removeChild(child):
:	Remove the child item specified from this item's children list. The item speicified must be a direct child of this item.
select():
:	Select this item, i.e. give it focus.

## Members
text:
:	The text label of the item, as displayed in the tree view of the dialog box
value:
:	The value associated with this item. This value is never displayed; it is used to easily recognize this item, and is also returned when requesting the currently selected value(s). It can be any python object, but it shouldn't be None (otherwise you won't be able to make the difference between when this item is selected and when nothing is selected).
checked:
:	For multi-selection tree views with checkboxes, indicate whether or not this item is currently checked.
partiallyChecked:
:	For multi-selection tree views with checkboxes, indicate whether or not this item is partially checked. 
expanded:
:	Indicates if this item is currently expanded (True) or collapsed (False).
hasChildNodes (read only):
:	Indicates if this item has one or more child nodes (True) or if it is a leaf node (False).
parentNode (read only):
:	Contains a reference to the parent of this item. This member is None only for the root element.
childNodes (read only):
:	Give a list of all children under this item. If this node has no children, the list is empty (and never None).
firstChild:
:	Contains a reference to the first child node of this item, or None if this item has no children.
lastChild:
:	Contains a reference to the last child node of this item, or None if this item has no children.
nextSibling:
:	Contains a reference to the next sibling item of this item, i.e. the next brother; or None if this item is the last of its brotherhood.
previousSibling:
:	Contains a reference to the previous sibling item of this item, i.e. the previous brother; or None if this item is the first of its brotherhood.

# ProgressDialog class
A progress dialog is used to show the progress of a lengthy task. To open a new progress dialog, you must use the `open` static method described below.
be careful, this must always be done in a background thread and never in the main UI thread of the application, otherwise the whole interface freezes.

## static methods
open(title='', text='') -> ProgressDialog:
:	Open a new progress dialog and show it, initially having the title and the text specified.

## Methods
close()
:	Close the progress dialog.

## Members
closed (Read only):
:	Indicates if the progress is closed or not.
cancelled (read only):
:	Indicate whether or not the user pressed the Cancel button, requesting an immediate cancellation of the task as soon as possible.
paused (read only):
:	Indicate whether or not the Pause button is currently pressed, requesting a pause in the processing of the task.
text:
:	The text currently displayed in the dialog box, above the progress bar.
value:
:	The value indicating the progress of the task, between 0 (nothing done) to 100 (finished)
