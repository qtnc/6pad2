from os import path
from . import FileType

class PythonFileType(FileType):
	pass

FileType.extensions(PythonFileType, ('py',))