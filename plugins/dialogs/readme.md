# Process32 python extension module
This python extension adress two issues that the standard *subprocess* python module has :

* It sometimes keepds the window of DOS process open
* It doesn't always work on windows, see <https://bugs.python.org/issue3905>

# The Process object

Constructor: Process(command, [encoding, binary])
- Command: the command to execute
- encoding: the encoding of the input/output; by default UTF-8
- binary: if True, bytes are returned instead of strings when reading

read([length]):
- Without any length specified: read all contents currently present on the pipe and return it
- With a given length specified: try to read exactly that number of bytes.

readline():
Read a line of text from the input pipe.

write(strOrBytes):
WRite the contents of the given str or bytes on the pipe.

flush():
Flush the contents of the write pipe.

wait():
Wait for the process to finish its execution.

close([errcode]):
- Without any argument: wait for the process to finish its execution and return its error code
- With an argument: immediately terminate the process, returning the error code specified.
