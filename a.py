import os, sys
import sixpad
from sixpad import window as win
from time import sleep
from threading import Thread
import qc6paddlgs as dlgs

def func():
	win.beep(800,120)

def func5 () :
	ppd = dlgs.ProgressDialog.open(title='Title', text='Text')
	for i in range(1,100):
		sleep(0.4)
		ppd.value=i
	ppd.close()

def func4 ():
	Thread(target=func5) .start()

def func3 (dlg):
	for i in range(1,11):
		item = dlg.root.appendChild('Item '+str(i), i*1000)
		for j in range(1,11):
			item.appendChild('Item ' + str(i) + '.' + str(j), 1000*i+j)

def func2 ():
	dlg = dlgs.TreeViewDialog.open(title='TreeViewDialog', hint='Example', modal=True, callback=func3)
	print(dlg)

#win.addAccelerator('F5', func)
win.addAccelerator('Ctrl+E', func4)
win.addAccelerator('Ctrl+0', func5)

#win.menus.tools.add(label='Hello item', accelerator='Ctrl+E', specific=True, action=func)
test = win.menus.add(label='&Test', name='test', index=4, submenu=True, specific=True)
for i in range(1,6):
	test.add(label='Item '+str(i), action=func, accelerator='Ctrl+'+str(i))

