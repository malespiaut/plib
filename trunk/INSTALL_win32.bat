@echo off

mkdir \usr >nul
mkdir \usr\include >nul
mkdir \usr\include\plib >nul
mkdir \usr\lib >nul

copy src\fnt\*.h \usr\include\plib
copy src\js\*.h \usr\include\plib
copy src\net\*.h \usr\include\plib
copy src\os\*.h \usr\include\plib
copy src\pui\*.h \usr\include\plib
copy src\sg\*.h \usr\include\plib
copy src\ssg\*.h \usr\include\plib
copy src\sl\*.h \usr\include\plib
copy src\util\*.h \usr\include\plib
copy src\net\*.h \usr\include\plib

copy src\fnt\debug\*.lib \usr\lib
copy src\net\debug\*.lib \usr\lib
copy src\pui\debug\*.lib \usr\lib
copy src\sg\debug\*.lib \usr\lib
copy src\ssg\debug\*.lib \usr\lib
copy src\sl\debug\*.lib \usr\lib
copy src\util\debug\*.lib \usr\lib
copy src\net\debug\*.lib \usr\lib
