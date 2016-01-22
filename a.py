import os, sys
import sixpad
from sixpad import window as win
from time import sleep
from threading import Thread
import qc6paddlgs as dlgs

def func(*args, **kwds):
	win.beep(800,120)

def func5 () :
	ppd = dlgs.ProgressDialog.open(title='Title', text='Text')
	for i in range(1,100):
		sleep(0.4)
		ppd.value=i
	ppd.close()

def func4 ():
	Thread(target=func5) .start()

def func3c (dlg, item):
	win.beep(item.checked*800+800, 200)

def func3 (dlg):
	dlg.addEvent('select', func3c)
	for i in range(1,10):
		item = dlg.root.appendChild('Item '+str(i), i*100)
		for j in range(1,10):
			subitem = item.appendChild('Item ' + str(i) + '.' + str(j), 100*i+10*j)
			for k in range(1,10):
				subsubitem = subitem.appendChild('Item ' + str(i) + '.' + str(j) + '.' + str(k), 100*i+10*j+k)

def func2 ():
	dlg = dlgs.TreeViewDialog.open(title='TreeViewDialog', hint='Example', modal=True, checkboxes=True, callback=func3)

#win.addAccelerator('F5', func)
win.addAccelerator('Ctrl+E', func4)
win.addAccelerator('Ctrl+0', func2)

#win.menus.tools.add(label='Hello item', accelerator='Ctrl+E', specific=True, action=func)
test = win.menus.add(label='&Test', name='test', index=4, submenu=True, specific=True)
for i in range(1,6):
	test.add(label='Item '+str(i), action=func, accelerator='Ctrl+'+str(i))

