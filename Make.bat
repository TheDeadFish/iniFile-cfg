call egcc.bat
gcc %CCFLAGS2% iniFile.cc -c
gcc %CCFLAGS2% test.cc iniFile.o -lstdshit
copy /Y iniFile.h %PROGRAMS%\local\include
ar -rcs  %PROGRAMS%\local\lib32\libexshit.a iniFile.o
