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

def func3 (dlg):
	for i in range(1,10):
		item = dlg.root.appendChild(text='Item '+str(i), value=i*100, checked=(i==1))
		for j in range(1,10):
			subitem = item.appendChild('Item ' + str(i) + '.' + str(j), 100*i+10*j, checked=j==1)
			for k in range(1,10):
				subsubitem = subitem.appendChild('Item ' + str(i) + '.' + str(j) + '.' + str(k), 100*i+10*j+k, checked=k==1)

def func2 ():
	dlg = dlgs.TreeViewDialog.open(title='TreeViewDialog', hint='Example', modal=True, multiple=True, editable=True, callback=func3)
	print(dlg)

def func6():
	print(win.taskDialog(
			title="Task dialog example",
			heading="Do you want to save changes ?",
			text="If you don't save, all modifications will be lost",
			checkbox="Don't show this dialog again",
			checked=False,
			icon='warning',
			buttons=("Save changes", "Don't save", "Randomly save"),
			radioButtons=('One', 'Two', 'Three', 'Four'),
			defaultRadioButton=2,
			defaultButton=2,
			footer='Footer text',
			details="There is no detail to show !",
			collapseButtonText='<< Details',
			expandButtonText='Details >>',
			commandLinks=True,
			commandLinksNoIcon=True
	))

#win.addAccelerator('F5', func)
win.addAccelerator('Ctrl+E', func6)
win.addAccelerator('Ctrl+0', func2)

#win.menus.tools.add(label='Hello item', accelerator='Ctrl+E', specific=True, action=func)
test = win.menus.add(label='&Test', name='test', index=4, submenu=True, specific=True)
for i in range(1,6):
	test.add(label='Item '+str(i), action=func, accelerator='Ctrl+'+str(i))

