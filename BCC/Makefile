SRC = ..\src\*.cpp 
INCLUDES = ..\include\*.h ..\h\*.h 
LIBS = ..\lib\*.lib 
INCLUDEPATH = C:\bc31\include;..\include;..\h;
LIBSPATH = C:\bc31\lib;..\lib;
TARGET = Kernel.exe
$(TARGET):	$(SRC) $(INCLUDES) configuration.xmi
	bcc.exe -mh -v -e$(TARGET) -I$(INCLUDEPATH) -L$(LIBSPATH) $(SRC) $(LIBS)
all:	$(TARGET)
clean:
	-del *.obj
	-del $(TARGET)
	-del *.asm
configuration.xmi:	
	configuration.xmi