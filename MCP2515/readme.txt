https://github.com/franksmicro/Arduino/tree/master/libraries/MCP2515


I was looking for a library to handle CAN on the Arduino using the MCP2515 
chip (such as is found in the Sparkfun CAN shield).  The only library I could
find was not in English, so I decided to write a library myself.

I have tested this library using the example OBDMPG program.  This program
successfully displays gas mileage statistics for my OBD2-compatible car on
the serial LCD.  It should be easy enough to change the sample program to
display other OBD parameters.  If your car uses a different baud rate than
500k, you will need to change this as well.

I can't promise support for this library, but feel free to E-mail me if
you have a question or see a problem.  If you make improvements, feel free
to contact me and I will consider merging your changes into the library,
with appropriate credit.

Frank

Modified by hooovahh in September, 2012 to fix bugs in extended addressing
and allow single variable for both regular and extended addresses.
