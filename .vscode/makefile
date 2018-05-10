SRCDIR		= src
HDIR		= h
INCLUDEDIR	= include
LIBDIR		= lib
ASMDIR		= asm
OBJDIR		= obj
OUTPUTDIR	= output
MODEL		= h
TARGETNAME	= Kernel.exe
TARGETARGS	= 

CDIR		= C:\bc31
CINCLUDEDIR	= $(CDIR)\include
CLIBDIR		= $(CDIR)\lib
CC			= $(CDIR)\bin\bcc
CFLAGS		= /c /m$(MODEL) /v /y /w /I$(HDIR);$(INCLUDEDIR);$(CINCLUDEDIR) /L$(LIBDIR);$(CLIBDIR)
TLINK		= $(CDIR)\bin\tlink
TLINKFLAGS	= /c /l /v /x /L$(CLIBDIR)
TD			= $(CDIR)\bin\td
TDFLAGS		= /c$(CDIR)\bin\tdconfig.td

SRC			= $(wildcard $(SRCDIR)/*.cpp)
H			= $(wildcard $(HDIR)/*.h)
ASM			= $(patsubst $(SRCDIR)/%.cpp,$(ASMDIR)/%.asm,$(SRC))
OBJ			= $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.obj,$(SRC))
LIB			= $(wildcard $(LIBDIR)/*.lib)
TARGET		= $(OUTPUTDIR)/$(TARGETNAME)
WINOBJ		= $(subst /,\,$(OBJ))
WINLIB		= $(subst /,\,$(LIB))
WINTARGET	= $(subst /,\,$(TARGET))

$(TARGET): $(OBJ)
	@-md $(OUTPUTDIR) 2>nul&
	@call $(TLINK) $(TLINKFLAGS) c0$(MODEL) $(WINOBJ), $(WINTARGET), , $(WINLIB) c$(MODEL) emu math$(MODEL)

$(OBJDIR)/%.obj: $(SRCDIR)/%.cpp $(H)
	@-md $(OBJDIR) 2>nul&
	@call $(CC) $(CFLAGS) /o$(subst /,\,$@) $(subst /,\,$<)

$(ASMDIR)/%.asm: $(SRCDIR)/%.cpp
	@-md $(ASMDIR) 2>nul&
	@call $(CC) /S $(CFLAGS) /o$(subst /,\,$@) $(subst /,\,$<)

compile: $(OBJ)
	@echo Compile finished!

all: $(TARGET)
	@echo "$(subst .exe,,$(TARGETNAME))" is up to date!

asm: $(ASM)
	@echo Generated assembly information for "$(subst .exe,,$(TARGETNAME))" is up to date!

@cleanasm:
	@-rd /s /q $(ASMDIR) 2>nul&

cleanasm: @cleanasm
	@echo Clean assembly succeeded!

@clean: @cleanasm
	@-rd /s /q $(OBJDIR) 2>nul&
	@-rd /s /q $(OUTPUTDIR) 2>nul&
	@-del *.$$LN 2>nul&

clean: @clean
	@echo Clean succeeded!

run:
	@call start cmd /c "$(WINTARGET) $(TARGETARGS) && pause"

debug:
	@call start cmd /c "$(TD) $(TDFLAGS) $(WINTARGET) $(TARGETARGS)"

.PHONY: compile all asm @cleanasm cleanasm @clean clean run debug