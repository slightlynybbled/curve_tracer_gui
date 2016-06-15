# Project Repositories #

Be sure to check out the [firmware](https://github.com/slightlynybbled/curve_tracer_firmware) and 
[hardware](https://github.com/slightlynybbled/curve_tracer_hardware) repositories!

# Running This Program #

## With Python 3 Installed ##

### Linux ###

On linux distributions, you may need to install additional dependencies.  For
instance, on the Rasberry Pi (Debian-based):

    sudo apt-get install python3-dev python3-setuptools python-imaging
    sudo apt-get install libjpeg-dev libtiff4-dev zlib1g-dev libfreetype6-dev libwebp-dev
    sudo apt-get install tcl-dev tk-dev
    
I realize that all of this can be installed in one run.  Go for it.

### Setup ###

Install Python 3.  At this time, there is no support for Python 2.  I imagine
that getting Python 2 support is trivial, I just don't want to do it and I 
feel that the community as a whole is moving to v3.

    pip install serialdispatch
    pip install pillow
    
or

    pip3 install serialdispatch
    pip3 install pillow
    
### Running ###

Execute the application with the python interpreter:

    python app.py
    
or

    python3 app.py
    

    

