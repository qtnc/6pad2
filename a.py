import os, sys
import sixpad
from sixpad import window as win

def pageOpened (p):
 pass

win.addEvent('pageOpened', pageOpened)
for page in win.pages: pageOpened(page)
