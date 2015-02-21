import window
import sys

class ConsoleRedirect:
	def write (self, text):
		window.print(text)

_console = ConsoleRedirect()
sys.stderr = _console
sys.stdout = _console
