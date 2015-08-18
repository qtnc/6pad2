import os, sys
import sixpad
from sixpad import window as win

def func () :
	win.beep(800,200)

def func2 ():
	print(win.menus.test)

win.addAccelerator('Ctrl+E', func2)

win.menus.tools.add(label='Hello item', accelerator='Ctrl+1', specific=True, action=func)
test = win.menus.add(label='&Test', name='test', index=4, submenu=True, specific=True)
for i in range(1,6):
	test.add(label='Item '+str(i), action=func)

