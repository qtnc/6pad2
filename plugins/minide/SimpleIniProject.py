from os import path
from . import Project

class SimpleIniProject (Project):
	def __init__ (self, id, dir, file):
		Project.__init__(self, id, dir)
		self.file = file


	def deploy ():
		pass

@Project.detector
def detector (dir):
	file = dir + '\\project.ini'
	if path.isfile(file): return SimpleIniProject(file, dir, file)

