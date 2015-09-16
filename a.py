import os, sys
import sixpad
from sixpad import window as win

import qc6paddlgs as dlgs

def func (dlg, key) :
	if key != 67+512: return True
	win.beep(800,200)
	return False

def func3 (dlg, item):
	win.beep(800,120)

def func2 ():
	dlgs.TreeViewDialog.close()
	dlg = dlgs.test()
	dlg.addEvent('keyDown', func)
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

