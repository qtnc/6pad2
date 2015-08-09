import os, sys
import sixpad
from sixpad import window as win

def test () :
	print(win.prompt(title='Test Input', prompt='Entrez votre nom', text='Yoyo', list=['One', 'Two', 'Three']))

def pageOpened (p):
 pass

win.addEvent('pageOpened', pageOpened)
for page in win.pages: pageOpened(page)

win.addAccelerator('Ctrl+E', test)
