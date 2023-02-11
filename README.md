# TiltFive CHOP for TouchDesigner

A work in progress C++ connector for the TiltFive NDK (Currently 1.3.0). This 
project implements the TiltFive NDK obtained from the 
[SDK download](https://files.tiltfive.com/Tilt%20Five%20SDK%20Setup%20-%201.3.0.exe). 
TiltFive is currently supported on Windows OS only. 

/include dir contains header files from the T5 NDK
/lib file contains DLLs from T5 NDK - Ensure DLL properties are set to Exclude File from Build - No, and
Item Type - Copy File

.toe file is a basic implementation of the C++ Operator in TouchDesigner.

## Issues

- Toggle requires disable > enable > disable > enable to properly init the Glasses. Default value for toggle not disabled at start. std::async doesn't wait for first init of Glasses to report back correct data