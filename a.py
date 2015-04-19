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

class UndoState:
	def undo (self, tab):
		window.beep(1600,200)
	def redo (self, tab):
		window.beep(3200,200)


def func () :
	o = UndoState()
	window.curPage.pushUndoState(o)

window.menus.edit.add(label='Item inutile', accelerator='Ctrl+E', action=func)


