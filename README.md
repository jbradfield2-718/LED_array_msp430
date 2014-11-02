LED Array Project
========================

Bare-Bones RPN Calculator Project, used to learn some of the functionality of the PyQt framework.
The calculator has a number of bugs, although the following functionality is supported:

 - Conversion between polar/rectangular coordinates.
 - Some complex number calculations are supported.
 - Standard scientific/trig functions (sin, cos, tan, log, ln, sqrt , !, mod, exponents, etc.).
 - Some standard scientific constants built in, and user-defined constants available.
 - Stack memory on restart of program.
 - Fixed digit, scientific, and engineering result display modes.
 - Small manual with basic explanation of the various modes and user entry.
 
Note that the file main.py contains the main stack logic; this *is not* the main program file.
To run the program, call rpn_calc_gui.py within the folder the main.py, display_gui.py, functions.py,
and config.py reside in.