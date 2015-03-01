import window
def func () :
	window.beep(4*392,220)
	window.beep(4*293,220)
	window.beep(4*494,220)
	window.beep(4*392,220)

window.addAccelerator('Ctrl+E', func)

o = window.MyObj()
print(o)
o.val += 7
o.val **= 2
print(o.val+2)
