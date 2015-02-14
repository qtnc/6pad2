# 6pad++, the successor of 6pad

6pad++ will be the successor of 6pad; 6pad is now abandonned.  
6pad old repository http://github.com/qtnc/6pad

The current version isn't ready for proper usage. Python is linked but nothing is done with it at the moment.  
Stay tuned !

# Changes from 6pad

Main changes compared to 6pad are:

* 6pad++ is in C++ instead of C
* 6pad++ will be scriptable in python 3.4 instead of lua 5.1.4/luajit 2.x
* Dropped PCRE API in favor of boost::regex

# Latest introduced or re-introduced features

* 24.01.2015: Tab system with independant text areas and properties (encoding, line ending, indentation, auto line wrap)
* 07.02.2015: Smart paste, taking care of indentation
* 07.02.2015: Ctrl+C/X when no selection => copy/cut current line

***END***