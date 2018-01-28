# 6pad++, the tiny and smart text editor

6pad++ is a simple and lightweight, yet powerful text editor for windows.
Its main goal is to keep the accessibility and responsiveness of standard notepad, while providing a few not especially spectacular but useful features.

# This project is non longer maintained
This project is non longer maintained and kept here only for archive. Once more, I started it again from the beginning.

C++ is quite a complex language, and integrating a scripting API in python into C++ isn't that easy. I was also directly using Win32API for the GUI. Win32API itself is quite complex without mentioning C++, and nowadays, there is more and more demand for multiplatform support.

Therefore, I decided to switch entirely to python, and use WXPython for the GUI. 
I hope that using a simpler language and toolbox will solve a serie of unknown bugs that are still present after many years, and encourage contributions.

Check out my new project: [Jane: just another editor](http://github.com/qtnc/jane).

# Download

You can download latest version at <http://vrac.quentinc.net/6pad%2b%2b.zip>.
Note that this is still an *alpha release* !

# Features

Main distinctive features of 6pad++ includes :

* Edit multiple files in a tabbed interface, and/or in multiple instances of the program, as you wish
* Manage most popular character encodings as well as less popular ones; up to 150 if your version of windows supports them all.
* Search and replace using perl-like regular expressions (provided by boost::regex).
* Open files at lightning speed, even for big files of a few dozends megabytes.
* Prevent opening a file twice in different tabs or windows, and automatically refereshing file's contents when modified in another external program.
* Use pipes to quickly read results of DOS command, or simply send them input without making temporary files
* Customize and extend the editor to fit your needs and liking, with a fully functional python 3 scripting API.

# Story

Since a long time, I have been frustrated by default windows' notepad because of its lake of functionalities for developers.
There are of course dedicated text editors especially made for developers, but I wheither find them too heavy, or not as accessible as I would like with a screen reader.

For example, notepad2 and notepad++ are very lightweight, but only partially accessible. Screen readers don't always behave as they should, or don't always read what they need to.
IN the opposite side, complete integrated developement editors like eclipse are known to be not so badly accessible, but they are most of the time too heavy, take time to start up, and aren't that suited to have a quick look at small files and other notes.

There effectively already exist a text editor made for screen reader users, it is called [EdSharp](http://empowermentzone.com/EdSharp.htm).
However, I find its interface not as easy as it is said; menus are especially full of rarely used features, are quite randomly mixed up, and it lakes an obvious possibility of customizing the whole thing with scripts in an easy way. And, of course, I don't program in C#.

# 6pad++, the successor of 6pad

6pad++ is the successor of 6pad. 6pad was programmed in pur C. It was often buggy because of this and didn't use a modern development language, that's why I decided to start a new project from the beginning, in C++14.
6pad proposed a lua scripting interface instead of python. I decided to switch to python because lua isn't very adapted in advanced string manipulation, after all the main task of a text editor. In particular, lua doesn't support unicode and UTF-8 natively.
Python 3 isn't a string manipulation specific language either, but it is today a widely popular language which perfectly supports Unicode and UTF-8. 
It also natively offers much more functionalities often useful as scripting language in a text editor beside text manipulation, such as file search for example.

You can have a look at the [old 6pad repository](http://github.com/qtnc/6pad) if you wish. That old 6pad is no longer maintained.

# Code organization

* In the *core* directory, is the code for the 6pad++ DLL (qc6pad10.dll). This is separated from the main executable so that plugins can be written in C++. If you want to write a C++ plugin for 6pad++, you need to import this library.
* IN the *main* directory is the code of the main 6pad++ executable.
* IN *doc* folder, are documentations about the final product and the associated python API. This documentation is auto-generated from markdown files using pandoc.
* In *lib* folder, are python libraries or DLLs, needed for the python 3.4.2 runtime.
*In *plugins* subfolder, are 6pad++ plugins programmed in python and/or C++ and their associated source code. Read each subfolder's readme for more info.
* IN *extensions* folder, are python extensions; they are completely independent and might be used outside 6pad++. Read each subfolder's readme for more info.

Note 1: 6pad++ binary is compiled with MinGW/GCC in 32-bit mode. If you want to make C++ plugins that can be used seamlessly with the base distribution, you must use that exact same compiler and settings.
This is because Visual Studio's and GCC's ABI aren't eachother compatible.

Note 2: 6pad++ binary is linked with msvcr100.dll instead of msvcrt.dll. This is an obligation because python34.dll has also been linked with msvcr100.dll and I don't have any control on python34.dll. This python34.dll, as well as the whole python library found in python34.zip, have been taken unmodified from the default python 3.4 windows installer available at <http://www.python.org/>. I just removed completely useless python modules.

# License

6pad++ is distributed as a free software with the GPL license version 3. For more information, please read license.txt.
