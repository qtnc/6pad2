import window
def func () :
	window.beep(4*392,220)
	window.beep(4*293,220)
	window.beep(4*494,220)
	window.beep(4*392,220)

mb = window.getMenuBar()
print(mb.file.exit)
sb = mb.file.add(submenu=True, index=-2, label='My submenu')
beep = sb.add(label='&Beep!', accelerator='Ctrl+E', index=-1, action=func)
beep.accelerator = 'Ctrl+3'
