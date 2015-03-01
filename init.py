import sys, window, traceback

class Console:
	def write (self, text):
		window.print(text)
	def print (self, text):
		window.print(text)
	def readline (self):
		return window.ConsoleReadImpl()

console = Console()
sys.stderr = console
sys.stdout = console
sys.stdin = console
window.console = console
