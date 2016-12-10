import os, sys
import sixpad
from sixpad import window as win
from time import sleep
from threading import Thread
import qc6paddlgs as dlgs

def fBeep (*args, **kwds):
	win.beep(800,120)

def fProgressCb (*args, **kwargs):
	pass

def fProgress () :
	win.beep(800,100)
	ptd = win.taskDialog(progressBar=True, title='ProgressBar Test', heading='Traitement en cours...', footer='Initialisation...', buttons=('Annuler',), callback=fProgressCb )
	value=0
	for i in range(1, 1000):
		if ptd.closed: break
		sleep(0.05)
		value=i
		ptd.value=i/1000.0
		ptd.footer = 'El\u00E9ment ' + str(i) + ' de 1000'
	print(value, ptd.buttonClicked)
	ptd.close()

def fProgressThd () :
	win.beep(400,100)
	t = Thread(fProgress)
	t.start()

def fListDlg ():
	win.choice( 'Test1', 'Test2', ['Item 1', 'Item 2', 'Item 3', 'Item 4', 'Item 5'] )
	lb = dlgs.ListBoxDialog.open('Some text 1', 'Some text 2', searchField=True)
	def action(*args, **kwargs):
		print(args, kwargs)
	for i in ('One', 'Two', 'Three', 'Four', 'Five', 'Six', 'Seven', 'Eight', 'Nine', 'Ten'): lb.append(i)
	lb.addEvent('action', action)

def fTreeDlgFill (dlg):
	for i in range(1,10):
		item = dlg.root.appendChild(text='Item '+str(i), value=i*100, checked=(i==1))
		for j in range(1,10):
			subitem = item.appendChild('Item ' + str(i) + '.' + str(j), 100*i+10*j, checked=j==1)
			for k in range(1,10):
				subsubitem = subitem.appendChild('Item ' + str(i) + '.' + str(j) + '.' + str(k), 100*i+10*j+k, checked=k==1)

def fTreeDlg ():
	dlg = dlgs.TreeViewDialog.open(title='TreeViewDialog', hint='Example', modal=True, multiple=True, editable=True, callback=fTreeDlgFill)
	print(dlg)

def fTaskDlg ():
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

def oenter (p, *args, **kwargs):
	return str(1+p.curLine) + ': '

def opo (p):
	p.addEvent('enter', oenter)
	menu = win.menus.tools.add(label='My SubMenu', submenu=True, group='testGroup', name='MenuSubGroup')
	menu.add(label='Specific item for page '+str(p), specific=True, action=fBeep, accelerator='Ctrl+9')
	if menu.length<3:
		menu.add(label='Group item test 1 '+str(p), action=fBeep, accelerator='Ctrl+0')
		menu.add(label='Group item test 2 '+str(p), action=fBeep, accelerator='Ctrl+8')
	win.beep(800,200)

win.addEvent('pageOpened', opo)


for i  in [
	{ 'label': 'Simple beep', 'action': fBeep, 'accelerator': 'Ctrl+1' },
	{ 'label': 'Progress dialog', 'action': fProgressThd, 'accelerator': 'Ctrl+2'  },
	{ 'label': 'ListBox dialog', 'action': fListDlg, 'accelerator': 'Ctrl+3'  },
	{ 'label': 'TreeView dialog', 'action': fTreeDlg, 'accelerator': 'Ctrl+4'  },
	{ 'label': 'Task dialog', 'action': fTaskDlg, 'accelerator': 'Ctrl+5' },
] :
	win.menus.tools.add(**i)
