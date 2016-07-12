# Purpose #

This directory is utilized in order to create a semi-uniform method for controlling
the build process for the curve tracer GUI.  Eventually, I expect this directory to
contain a spec file that should work on any platform.  For now, I expect this directory
to contain one or two platforms that I happen to own and can test myself.

# Use #

## Installing PyInstaller ##

The user must first install pyinstaller:

`pip install pyinstaller`

There are probably a half-dozen other ways to get this going, but
you will be familiar with those if you have a special setup.

## Creating the executable ##

On the command-line, navigate to this directory and execute:

`pyinstaller app.spec`

The executable will be located in the "dist" directory.
