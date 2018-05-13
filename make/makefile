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
	@-md $(WPREPPATH) 2>nul&
	@call $(CPP) $(CPPFLAGS) /o$$(call unix-to-dos,$$@) $$(call unix-to-dos,$$<)
$(call src-to-asm,$1): $1 $(H)
	@-md $(WASMPATH) 2>nul&
	@call $(CXX) /S $(CXXFLAGS) /o$$(call unix-to-dos,$$@) $$(call unix-to-dos,$$<)
$(call src-to-obj,$1): $1 $(H)
	@-md $(WOBJPATH) 2>nul&
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

WSRCPATH		:= $(call unix-to-dos,$(SRCPATH))
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
WCINCLUDEPATH	:= $(WCPATH)\include
WCLIBPATH		:= $(WCPATH)\lib
CPP				:= $(WCPATH)\bin\cpp.exe
CPPFLAGS		:= /m$(MEMMODEL) /w /I$(WHPATH);$(WINCLUDEPATH);$(WCINCLUDEPATH)
CXX				:= $(WCPATH)\bin\bcc.exe
CXXFLAGS		:= /c /m$(MEMMODEL) /v /y /w /I$(WHPATH);$(WINCLUDEPATH);$(WCINCLUDEPATH) /L$(WLIBPATH);$(WCLIBPATH)
TLINK			:= $(WCPATH)\bin\tlink.exe
TLINKFLAGS		:= /c /l /v /x /L$(WCLIBPATH)
TD				:= $(WCPATH)\bin\td.exe
TDFLAGS			:= /c$(WCPATH)\bin\tdconfig.td

# Internal generated variables. You should not change these.
SRC				:= $(strip $(wildcard $(SRCPATH)/*.cpp) $(wildcard $(SRCPATH)/*.CPP))
H				:= $(strip $(wildcard $(HPATH)/*.h) $(wildcard $(HPATH)/*.H))
LIB				:= $(strip $(wildcard $(LIBPATH)/*.lib) $(wildcard $(LIBPATH)/*.LIB))
PREP			:= $(call src-to-prep,$(SRC))
ASM				:= $(call src-to-asm,$(SRC))
OBJ				:= $(call src-to-obj,$(SRC))
TARGET			:= $(OUTPUTPATH)/$(subst .EXE,,$(call uc,$(TARGETNAME))).EXE

WSRC			:= $(call unix-to-dos,$(SRC))
WH				:= $(call unix-to-dos,$(H))
WLIB			:= $(call unix-to-dos,$(LIB))
WPREP			:= $(call unix-to-dos,$(PREP))
WASM			:= $(call unix-to-dos,$(ASM))
WOBJ			:= $(call unix-to-dos,$(OBJ))
WTARGET			:= $(call unix-to-dos,$(TARGET))

$(TARGET): $(OBJ)
	@-md $(WOUTPUTPATH) 2>nul&
	@call $(TLINK) $(TLINKFLAGS) c0$(MEMMODEL) $(WOBJ), $(WTARGET), , $(WLIB) c$(MEMMODEL) emu math$(MEMMODEL)

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
	@-rd /s /q $(WPREPPATH) 2>nul&

cleanprep: @cleanprep
	@echo Successfully cleaned preprocessor information!

@cleanasm:
	@-rd /s /q $(WASMPATH) 2>nul&

cleanasm: @cleanasm
	@echo Successfully cleaned assembly information!

@cleanobj:
	@-rd /s /q $(WOBJPATH) 2>nul&

cleanobj: @cleanobj
	@echo Successfully cleaned object files!

@clean: @cleanprep @cleanasm @cleanobj
	@-rd /s /q $(WOUTPUTPATH) 2>nul&
	@-del *.$$LN 2>nul&

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
	$(info WSRCPATH is $(WSRCPATH))
	$(info WHPATH is $(WHPATH))
	$(info WINCLUDEPATH is $(WINCLUDEPATH))
	$(info WLIBPATH is $(WLIBPATH))
	$(info WPREPPATH is $(WPREPPATH))
	$(info WASMPATH is $(WASMPATH))
	$(info WOBJPATH is $(WOBJPATH))
	$(info WOUTPUTPATH is $(WOUTPUTPATH))
	$(info )
	$(info WCPATH is $(WCPATH))
	$(info WCINCLUDEPATH is $(WCINCLUDEPATH))
	$(info WCLIBPATH is $(WCLIBPATH))
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
	$(info WSRC is $(WSRC))
	$(info WH is $(WH))
	$(info WLIB is $(WLIB))
	$(info WPREP is $(WPREP))
	$(info WASM is $(WASM))
	$(info WOBJ is $(WOBJ))
	$(info WTARGET is $(WTARGET))
	$(info $(foreach d,$(SRC),$(call generate-rules,$d)))

.PHONY: prep asm compile all @cleanprep cleanprep @cleanasm cleanasm @cleanobj cleanobj @clean clean run debug debuginfo