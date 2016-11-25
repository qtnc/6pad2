# 6pad++ changelog

## 6pad++ Beta 1 (October 2015)
6pad++ is officially no longer at alpha stage and all changes will be documented in this file from now on.

## 6pad++ Alpha 8 (august 2016)
Working under windows 10
Fixed GitHub issues #2, #4 and #7
Known bugs: back references \1, etc. no longer work in regular expression search/replace. Should update boost but boost::variant no longer work after boost 1.55

## 6pad++ Alpha 11
Regular expression bug with back references (\1, etc.) solved
Bug with recent files not always appearing in the menu, or no recent file appearing at all solved
Replaced the notion of specific menu item by a concept of menus belonging to groups of items, allowing to share menus and menu items on several pages
When changing indentation mode, it now asks if existing indentations in the old style should be replaced to the new style