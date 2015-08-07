import os
from process32 import Process
from sixpad import say, msg, window as win

class Project:
	def __init__(self, dir):
		self.dir=dir
	
	def compile (self, page):
		oldDir = os.getcwd()
		os.chdir(self.dir)
		proc = Process('c.bat')
		proc.wait()
		result = proc.read()
		os.chdir(oldDir)
		if not result:
			win.messageBeep(64)
			return
		win.messageBeep(16)
		say(result)
		p = win.new('text')
		p.readOnly=True
		p.name = msg('Compilation result')
		p.text = result
