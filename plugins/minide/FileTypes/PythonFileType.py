from .. import FileType

@FileType.extensions( ('py', 'pyw') )
class PythonFileType(FileType):
	pass

