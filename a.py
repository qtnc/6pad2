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

def func (self) :
	window.beep(1700,100)
	print(self)

#window.menus.edit.add(label='Item inutile', accelerator='Ctrl+E', action=func)
window.addEvent('pageOpened', func)
print('Python OK, tabs=' + str(len(window.pages)) )