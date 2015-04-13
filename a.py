import sys
import sixpad
from sixpad import window

def func4 () :
	pop = window.createPopupMenu()
	i1 = pop.add(label='Item 1')
	i2 = pop.add(label='Item 2')
	i3 = pop.add(label='Item 3')
	print(pop)
	print(i1)
	print(i2)
	print(i3)
	re = pop.show()
	window.alert('Result=' + str(re), 'Info')

def f2 (self, line):
	print(line)
	return False

def func () :
	window.curPage.addEvent('enter', f2)


window.menus.edit.add(label='Item inutile', accelerator='Ctrl+E', action=func)

