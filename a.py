import window
import sys
def func () :
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

mb = window.getMenuBar()
sb = mb.file.add(submenu=True, index=-2, label='My submenu')
beep = sb.add(label='&Beep!', accelerator='Ctrl+E', index=-1, action=func)

