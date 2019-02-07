# Linux Terminal Launcher for WSL


This utility launches real linux terminals such as *gnome-terminal, tilix or xterm* through X11 providing a better terminal experience on Windows Subsystem on Linux.


**Prerequisites**:

+ A recent build of Windows 10 with WSL installed
+ At least one terminal emulator installed on the WSL Linux distro. For example you may install gnome-terminal on Ubuntu by `sudo apt install gnome-terminal`

+ Visual Studio 2015 or later. (For C++ 17 filesystem support)


**Build**:

+ The compiler should have C++ 17 support. 
+ The subsystem option would better be set to WINDOWS (/SUBSYSTEM:WINDOWS) to avoid a flashing window on start. 

* **You may simply use the preconfigured Visual C++ project file TerminalLauncher.vcxproj**



**Usage**:

```sh

Terminal

    -c disrto terminal     #Change WSL distro-launcher and terminal program

    -n cmdline             #Change launch command-line to cmdline

    -i [location]          #Install TerminalLauncher to Windows\System32

    -s                     #Show current command line
```
 
