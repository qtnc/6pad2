import os, sys
import sixpad
from sixpad import window as win

import qc6paddlgs as dlgs

def func () :
	win.beep(800,200)

def func3 (dlg, item):
	win.beep(800,120)

def func2 ():
	dlg = dlgs.test()
	dlg.addEvent('delete', func3)
	for i in range(1,11):
		item = dlg.root.appendChild('Item '+str(i))
		for j in range(1,11):
			item.appendChild('Item ' + str(i) + '.' + str(j))


#win.addAccelerator('F5', func)
win.addAccelerator('Ctrl+E', func2)

#win.menus.tools.add(label='Hello item', accelerator='Ctrl+E', specific=True, action=func)
test = win.menus.add(label='&Test', name='test', index=4, submenu=True, specific=True)
for i in range(1,6):
	test.add(label='Item '+str(i), action=func, accelerator='Ctrl+'+str(i))

