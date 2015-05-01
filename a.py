import os, sys
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

def func () :
	window.beep(1047,300)
	func2()

def func2() :
	window.beep(524,300)

window.menus.edit.add(name='test', label='Item inutile', accelerator='Ctrl+E', action=func)
window.setTimeout(func, 4000)
window.setTimeout(func2, 1000)
