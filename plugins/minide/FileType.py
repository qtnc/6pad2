class FileType:
	detectors = []
	
	def __init__ (self, file):
		self.file=file

	@staticmethod
	def ext (file):
		return file[1+file.rfind('.'):].lower() if file.find('.')>0 else ''
	
	@classmethod
	def detector (self, d):
		self.detectors.append(d); return d
	
	@classmethod
	def extensions (self, exts, *args, **kwargs):
		def f0 (cls):
			def f1 (file):
				if self.ext(file) in exts: return cls(file, *args, **kwargs)
			return self.detector(f1)
		return f0
