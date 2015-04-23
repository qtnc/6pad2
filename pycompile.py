import sys, py_compile

del sys.argv[0]
for arg in sys.argv:
	py_compile.compile(arg, arg+'o')
