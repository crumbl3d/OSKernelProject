# You can change these variables for your project.
# PROJECTDIR will be added to all other variables.
PROJECTDIR		:= ..
SRCDIR			:= src
HDIR			:= h
INCLUDEDIR		:= include
LIBDIR			:= lib
PREPDIR			:= prep
ASMDIR			:= asm
OBJDIR			:= obj
OUTPUTDIR		:= output

# Here you can change program parameters.
MEMMODEL		:= h
TARGETNAME		:= Kernel
TARGETARGS		:= 

# GNU Make Standard Library.
include gmsl

# Custom functions... DO NOT MODIFY!
unix-to-dos		= $(subst /,\,$1)
src-to-prep		= $(addprefix $(PREPPATH)/,$(call uc,$(patsubst %.cpp,%.i,$(call lc,$(notdir $1)))))
src-to-asm		= $(addprefix $(ASMPATH)/,$(call uc,$(patsubst %.cpp,%.asm,$(call lc,$(notdir $1)))))
src-to-obj		= $(addprefix $(OBJPATH)/,$(call uc,$(patsubst %.cpp,%.obj,$(call lc,$(notdir $1)))))

# After calling eval generates rules for all files.
define generate-rules
$(call src-to-prep,$1): $1 $(H)
	@-md $(WPREPPATH) 2> nul & ver > nul
	@call $(CPP) $(CPPFLAGS) /o$$(call unix-to-dos,$$@) $$(call unix-to-dos,$$<)
$(call src-to-asm,$1): $1 $(H)
	@-md $(WASMPATH) 2> nul & ver > nul
	@call $(CXX) /S $(CXXFLAGS) /o$$(call unix-to-dos,$$@) $$(call unix-to-dos,$$<)
$(call src-to-obj,$1): $1 $(H)
	@-md $(WOBJPATH) 2> nul & ver > nul
	@call $(CXX) $(CXXFLAGS) /o$$(call unix-to-dos,$$@) $$(call unix-to-dos,$$<)
endef

# Automatic path creation. You should not change these.
SRCPATH			:= $(PROJECTDIR)/$(SRCDIR)
HPATH			:= $(PROJECTDIR)/$(HDIR)
INCLUDEPATH		:= $(PROJECTDIR)/$(INCLUDEDIR)
LIBPATH			:= $(PROJECTDIR)/$(LIBDIR)
PREPPATH		:= $(PROJECTDIR)/$(PREPDIR)
ASMPATH			:= $(PROJECTDIR)/$(ASMDIR)
OBJPATH			:= $(PROJECTDIR)/$(OBJDIR)
OUTPUTPATH		:= $(PROJECTDIR)/$(OUTPUTDIR)

WHPATH			:= $(call unix-to-dos,$(HPATH))
WINCLUDEPATH	:= $(call unix-to-dos,$(INCLUDEPATH))
WLIBPATH		:= $(call unix-to-dos,$(LIBPATH))
WPREPPATH		:= $(call unix-to-dos,$(PREPPATH))
WASMPATH		:= $(call unix-to-dos,$(ASMPATH))
WOBJPATH		:= $(call unix-to-dos,$(OBJPATH))
WOUTPUTPATH		:= $(call unix-to-dos,$(OUTPUTPATH))

# Compiler and linker location and flags.
# You should only ever change WCPATH.
WCPATH			:= C:\bc31
WCBINPATH		:= $(WCPATH)\bin
WCINCLUDEPATH	:= $(WCPATH)\include
WCLIBPATH		:= $(WCPATH)\lib

CPP				:= $(WCBINPATH)\cpp.exe
CPPFLAGS		:= /m$(MEMMODEL) /w /I$(WHPATH);$(WINCLUDEPATH);$(WCINCLUDEPATH)
CXX				:= $(WCBINPATH)\bcc.exe
CXXFLAGS		:= /c /m$(MEMMODEL) /v /y /w /I$(WHPATH);$(WINCLUDEPATH);$(WCINCLUDEPATH) /L$(WLIBPATH);$(WCLIBPATH)
TLINK			:= $(WCBINPATH)\tlink.exe
TLINKFLAGS		:= /c /l /v /x
TD				:= $(WCBINPATH)\td.exe
TDFLAGS			:= /c$(WCBINPATH)\tdconfig.td

# Stripping extension for use in messages.
TARGETNAME		:= $(basename $(TARGETNAME))

# Internal generated variables. You should not change these.
SRC				:= $(strip $(wildcard $(SRCPATH)/*.cpp) $(wildcard $(SRCPATH)/*.CPP))
H				:= $(strip $(wildcard $(HPATH)/*.h) $(wildcard $(HPATH)/*.H))
LIB				:= $(strip $(wildcard $(LIBPATH)/*.lib) $(wildcard $(LIBPATH)/*.LIB))
PREP			:= $(call src-to-prep,$(SRC))
ASM				:= $(call src-to-asm,$(SRC))
OBJ				:= $(call src-to-obj,$(SRC))
TARGET			:= $(OUTPUTPATH)/$(call uc,$(TARGETNAME)).EXE

WTARGET			:= $(call unix-to-dos,$(TARGET))

M				:= $(call uc,$(MEMMODEL))
LIBNAMES		:= $(basename $(notdir $(LIB))) C$M EMU MATH$M
OBJNAMES		:= C0$M $(basename $(notdir $(OBJ)))

$(TARGET): $(OBJ)
	@-md $(WOUTPUTPATH) 2> nul & ver > nul
	@copy $(WOBJPATH)\*.OBJ $(WOUTPUTPATH) > nul
	@copy $(WLIBPATH)\*.LIB $(WOUTPUTPATH) > nul
	@copy $(WCLIBPATH)\C0$M.OBJ $(WOUTPUTPATH) > nul
	@copy $(WCLIBPATH)\C$M.LIB $(WOUTPUTPATH) > nul
	@copy $(WCLIBPATH)\EMU.LIB $(WOUTPUTPATH) > nul
	@copy $(WCLIBPATH)\MATH$M.LIB $(WOUTPUTPATH) > nul
	@-cd $(WOUTPUTPATH) 2> nul & \
	@call $(TLINK) $(TLINKFLAGS) $(OBJNAMES),$(TARGETNAME),,$(LIBNAMES)
	@-del $(WOUTPUTPATH)\*.OBJ 2> nul & ver > nul
	@-del $(WOUTPUTPATH)\*.LIB 2> nul & ver > nul

$(foreach d,$(SRC),$(eval $(call generate-rules,$d)))

prep: $(PREP)
	@echo Successfully generated preprocessor information for "$(TARGETNAME)"!

asm: $(ASM)
	@echo Successfully generated assembly information for "$(TARGETNAME)"!

compile: $(OBJ)
	@echo Successfully compiled "$(TARGETNAME)"!

all: $(TARGET)
	@echo "$(TARGETNAME)" is up to date!

@cleanprep:
	@-rd /s /q $(WPREPPATH) 2> nul & ver > nul

cleanprep: @cleanprep
	@echo Successfully cleaned preprocessor information!

@cleanasm:
	@-rd /s /q $(WASMPATH) 2> nul & ver > nul

cleanasm: @cleanasm
	@echo Successfully cleaned assembly information!

@cleanobj:
	@-rd /s /q $(WOBJPATH) 2> nul & ver > nul

cleanobj: @cleanobj
	@echo Successfully cleaned object files!

@clean: @cleanprep @cleanasm @cleanobj
	@-rd /s /q $(WOUTPUTPATH) 2> nul & ver > nul
	@-del *.$$LN 2> nul & ver > nul

clean: @clean
	@echo Successfully cleaned project!

run:
ifneq ("$(wildcard $(TARGET))","")
	@call start cmd /c "$(WTARGET) $(TARGETARGS) && pause"
else
	@echo You must build the project first before running it!
endif

debug:
ifneq ("$(wildcard $(TARGET))","")
	@call start cmd /c "$(TD) $(TDFLAGS) $(WTARGET) $(TARGETARGS)"
else
	@echo You must build the project first before debugging it!
endif

debuginfo:
	$(info PROJECTDIR is $(PROJECTDIR))
	$(info SRCDIR is $(SRCDIR))
	$(info HDIR is $(HDIR))
	$(info INCLUDEDIR is $(INCLUDEDIR))
	$(info LIBDIR is $(LIBDIR))
	$(info PREPDIR is $(PREPDIR))
	$(info ASMDIR is $(ASMDIR))
	$(info OBJDIR is $(OBJDIR))
	$(info OUTPUTDIR is $(OUTPUTDIR))
	$(info )
	$(info MEMMODEL is $(MEMMODEL))
	$(info TARGETNAME is $(TARGETNAME))
	$(info TARGETARGS is $(TARGETARGS))
	$(info )
	$(info SRCPATH is $(SRCPATH))
	$(info HPATH is $(HPATH))
	$(info INCLUDEPATH is $(INCLUDEPATH))
	$(info LIBPATH is $(LIBPATH))
	$(info PREPPATH is $(PREPPATH))
	$(info ASMPATH is $(ASMPATH))
	$(info OBJPATH is $(OBJPATH))
	$(info OUTPUTPATH is $(OUTPUTPATH))
	$(info )
	$(info WHPATH is $(WHPATH))
	$(info WINCLUDEPATH is $(WINCLUDEPATH))
	$(info WLIBPATH is $(WLIBPATH))
	$(info WPREPPATH is $(WPREPPATH))
	$(info WASMPATH is $(WASMPATH))
	$(info WOBJPATH is $(WOBJPATH))
	$(info WOUTPUTPATH is $(WOUTPUTPATH))
	$(info )
	$(info WCPATH is $(WCPATH))
	$(info WCBINPATH is $(WCBINPATH))
	$(info WCINCLUDEPATH is $(WCINCLUDEPATH))
	$(info WCLIBPATH is $(WCLIBPATH))
	$(info )
	$(info CPP is $(CPP))
	$(info CPPFLAGS is $(CPPFLAGS))
	$(info CXX is $(CXX))
	$(info CXXFLAGS is $(CXXFLAGS))
	$(info TLINK is $(TLINK))
	$(info TLINKFLAGS is $(TLINKFLAGS))
	$(info TD is $(TD))
	$(info TDFLAGS is $(TDFLAGS))
	$(info )
	$(info SRC is $(SRC))
	$(info H is $(H))
	$(info LIB is $(LIB))
	$(info PREP is $(PREP))
	$(info ASM is $(ASM))
	$(info OBJ is $(OBJ))
	$(info TARGET is $(TARGET))
	$(info )
	$(info WTARGET is $(WTARGET))
	$(info )
	$(info M is $(M))
	$(info LIBNAMES is $(LIBNAMES))
	$(info OBJNAMES is $(OBJNAMES))
	$(info )
	$(info $(foreach d,$(SRC),$(call generate-rules,$d)))

.PHONY: prep asm compile all @cleanprep cleanprep @cleanasm cleanasm @cleanobj cleanobj @clean clean run debug debuginfo