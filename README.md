robot_daemon - cross-platform UNIX-daemon, that can be used as a base for developing other daemons. Written on C++11.

License

Distributed under GNU GPL v2 license, contains code, distributed under MIT license (spdlog) and under Boost Software License - Version 1.0

Supported CPU architectures

amd64
x86 - unknown
arm - unknown


Dependencies

git >= 2.1.4
libconfig++ >= 9
libboost >= 1.59
cmake >= 3.0.2
g++ >= 4.8.2 (support C++11 standard)
Also it may be compiled with clang++ >= 3.6.0.
These programs need to be installed to generate documentation:

doxygen >= 1.8.9.1
mscgen >= 0.20
graphviz >= 2.38.0
To generate documentation in the pdf-format:

pdflatex >= 3.14159265-2.6-1.40.15
Older versions of the programs and libraries is not tested.

Compile and Install

1) Last version of the robotd software can be downloaded using git:

$ git clone https://github.com/aido93/robotd

2) Change directory to ./robotd:

$ cd ./robotd

3) Configure robotd:

$ cmake .

4) Build the project:

$ make



Run

By default, robotd use directories defined by the FHS: for log-files (/var/log/) and for temporary files, required for correct work (/var/run). Thereat, robotd needs to be run as root. If you want, you can modify the robotd code for using other directories and don't run it as root. How to do that is showed in the robotd structure. Below, it assumed that you prefer to run program as root.

To run robotd type this command in terminal:

$ sudo ./robotd load cfg/daemon.cfg

This command tells to the program to load standard configuration file, that placed in the ./cfg/ directory.

To stop robotd:

$ sudo ./robotd stop

If you want to see other options, you can type:

$ ./robotd help

<option> robotd 1.0 - cross-platform UNIX daemonized server. 
License GNU GPLv2 <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>
This program can be used for development your own daemon.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Written by Igor Diakonov. Compiled 23.01.2016.


Usage: robotd

As we can see, 'load' option requires one parameter - the name of the configuration file. It supports as full filenames or relative filenames.

Author
Diakonov Igor
Date
20.01.2016
