# 6pad++, the successor of 6pad

6pad++ is the successor of 6pad; 6pad is now abandonned.  
6pad old repository is at <http://github.com/qtnc/6pad>

The current version isn't ready for proper usage, not even a 0.1. Python is linked but nothing is done with it at the moment.  
Stay tuned !

# Changes from 6pad

Main changes compared to 6pad are:

* 6pad++ is in C++ instead of C
* 6pad++ is scriptable in python 3.4 instead of lua 5.1.4/luajit 2.x
* Dropped PCRE API in favor of boost::regex

# Organization

* IN this root directory, are the main C++ source code of 6pad++.
* IN *doc* folder, are documentations about the final product 6pad++.
* In *lib* folder, are python libraries or DLLs
*In *plugins* subfolder, are 6pad++ plugins programmed in python. Read each subfolder's readme for more info.
* In *addons* folder, are 6pad++ extension DLLs, and their associated source code. Read each subfolder's readme for more info.
* IN *extensions* folder, are python extensions; they might be used outside 6pad++. Read each subfolder's readme for more info.

