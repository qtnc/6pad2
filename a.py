import window
def func () :
	window.beep(4*392,220)
	window.beep(4*293,220)
	window.beep(4*494,220)
	window.beep(4*392,220)

mb = window.getMenuBar()
mb[0].name='file'
mb.file.add(label='&Beep!', accelerator='Ctrl+E', index=-2, action=func)

