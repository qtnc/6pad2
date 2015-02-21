import window
def func () :
	window.beep(4*392,220)
	window.beep(4*293,220)
	window.beep(4*494,220)
	window.beep(4*392,220)

re = window.addAccelerator('Ctrl+E', func)
print(re)

