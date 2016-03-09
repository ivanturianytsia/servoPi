# servoPi
Program for giving commands to a servomotor via text file. C++ code for Raspberry Pi with WiringPi lib. 

Has two modes: 
  - angle mode - exact angles are specified: [0; 180]deg
  - position mode - one of 5 possible positions: 0, 1, 2, 3, 4; Formula: (angle)=(position)*45deg

Mode must be specified in the begining of the file as 'angle' or 'position'.

Example command sheet can be found in servocmd.txt
