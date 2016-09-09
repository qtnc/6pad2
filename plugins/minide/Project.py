from os import path

class Project:
	detectors = []
	projects = {}
	
	@classmethod
	def detector (self, d):
		self.detectors.append(d); return d

	@classmethod
	def projectFile (self, filename, *args, **kwargs):
		def f0 (cls):
			def f1 (dir):
				file = dir+'\\'+filename
				if path.isfile(file): return cls(dir, file, *args, **kwargs)
			return self.detector(f1)
		return f0
	
	def __init__ (self, dir, file):
		self.dir=dir
		self.file = file
