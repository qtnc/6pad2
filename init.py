import sys, sixpad

sysRead = sixpad.sysRead
sysPrint = sixpad.sysPrint
sixpad.sysRead = None
sixpad.sysPrint = None

class Console:
	def write (self, text):
		sysPrint(text)
	def print (self, text):
		sysPrint(text)
	def readline (self):
		return sysRead()

console = Console()
sys.stderr = console
sys.stdout = console
sys.stdin = console
sixpad.console = console
