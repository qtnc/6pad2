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
	window.beep(785,150)
	window.beep(785,300)
	window.beep(1047,150)
	window.beep(524,300)


window.menus.edit.add(name='test', label='Item inutile', accelerator='Ctrl+E', action=func)

window.curPage.test('Salut, bonjour !')

