COMPILER:=g++
CPPFLAGS:=-Wall -Wextra -g -pedantic -std=c++17 -masm=intel
TARGET:=saboteur
TARGETTEST:=saboteurtest

# ----------
# Extensions

HPPCONST:=.hpp
CPPCONST:=.cpp
OBJCONST:=.o
GCHCONST:=.gch

ALLHPPCONST:=*$(HPPCONST)
ALLCPPCONST:=*$(CPPCONST)
ALLOBJCONST:=*$(OBJCONST)
ALLGCHCONST:=*$(PCHCONST)

# -----------
# Directories

BIN_DIR:=bin
INCLUDE_DIR:=include
OBJ_DIR:=obj
SOURCE_DIR:=src
TEST_DIR:=test
SABOTEUR_DIR:=saboteur
INTERFACES_DIR:=interfaces

# -----
# Names

TYPES:=Types
SABOTEUROBSERVER:=SaboteurObserver
SABOTEUR:=Saboteur
SABOTEURATTRIBUTE:=SaboteurAttribute
NAMESPACE:=Opal

# ----------
# Root Paths

INCLUDEPATH:=-I$(INCLUDE_DIR)/
SOURCEPATH:=$(SOURCE_DIR)/

# -------------
# Include Paths

INTERFACESINCLUDEPATH:=$(INCLUDEPATH)$(INTERFACES_DIR)/
SABOTEURINCLUDEPATH:=$(INCLUDEPATH)$(SABOTEUR_DIR)/

# ----------
# File Paths

TYPESPATH:=$(INCLUDE_DIR)/$(TYPES)$(HPPCONST)
SABOTEUROBSERVERPATH:=$(INCLUDE_DIR)/$(INTERFACES_DIR)/$(SABOTEUROBSERVER)$(HPPCONST)
SABOTEURPATH:=$(INCLUDE_DIR)/$(SABOTEUR_DIR)/$(SABOTEUR)$(HPPCONST)
NAMESPACEPATH:=$(INCLUDE_DIR)/$(NAMESPACE)$(HPPCONST)

# -------------------
# Precompiled Headers

TYPES_GCH:=$(TYPESPATH)$(GCHCONST)
SABOTEUROBSERVER_GCH:=$(SABOTEUROBSERVERPATH)$(GCHCONST)
SABOTEUR_GCH:=$(SABOTEURPATH)$(GCHCONST)
NAMESPACE_GCH:=$(NAMESPACEPATH)$(GCHCONST)

# -------------------------------------
# Header Precompilation Build Arguments

TYPESBUILDARGS_GCH:=-c $(INCLUDEPATH) $(TYPESPATH) -o $(TYPES_GCH)
SABOTEUROBSERVERBUILDARGS_GCH:=-c $(SABOTEUROBSERVERPATH) -o $(SABOTEUROBSERVER_GCH)
SABOTEURBUILDARGS_GCH:=-c $(INCLUDEPATH) $(INTERFACESINCLUDEPATH) $(SABOTEURINCLUDEPATH) $(SABOTEURPATH) -o $(SABOTEUR_GCH)
NAMESPACEBUILDARGS_GCH:=-c $(INCLUDEPATH) $(INTERFACESINCLUDEPATH) $(SABOTEURINCLUDEPATH) $(NAMESPACEPATH) -o $(NAMESPACE_GCH)

# -----------
# Source Path

SABOTEUR_SOURCEPATH:=$(SOURCE_DIR)/$(SABOTEUR_DIR)/$(SABOTEUR)$(CPPCONST)

# -----------
# Object Path

SABOTEUR_OBJ:=$(OBJ_DIR)/$(SABOTEUR)$(OBJCONST)

# -------------------------------------
# Object Precompilation Build Arguments

SABOTEURBUILDARGS_OBJ:=-c $(SABOTEURINCLUDEPATH) $(SABOTEUR_SOURCEPATH) -o $(SABOTEUR_OBJ)

# -------------------
# Dependency Includes

DEPENDENCIES:=$(INCLUDEPATH) $(INTERFACESINCLUDEPATH) $(SABOTEURINCLUDEPATH)

# -------
# Modules

MODULES:=$(SABOTEUR_OBJ)

# -------
# Targets

$(BIN_DIR)/$(TARGET):
	clear
	@echo "Compiling..."
	@echo "Assembling Saboteur Lifecycle..."
	yasm -g dwarf2 -f elf64 src/assembly/x86_64/64_bit/linux/Saboteur.asm -l ./Lifecycle.lst -o Lifecycle.o
	@echo "Precompiling Headers"
	$(COMPILER) $(CPPFLAGS) $(TYPESBUILDARGS_GCH)
	$(COMPILER) $(CPPFLAGS) $(SABOTEUROBSERVERBUILDARGS_GCH)
	$(COMPILER) $(CPPFLAGS) $(SABOTEURBUILDARGS_GCH)
	$(COMPILER) $(CPPFLAGS) $(NAMESPACEBUILDARGS_GCH)
	@echo "Precompiling Modules"
	$(COMPILER) $(CPPFLAGS) $(SABOTEURBUILDARGS_OBJ)
	@echo "Compiling Main"
	$(COMPILER) $(CPPFLAGS) -no-pie $(DEPENDENCIES) Lifecycle.o -o $(BIN_DIR)/$(TARGET) $(SOURCEPATH)$(ALLCPPCONST) $(MODULES) -pthread

headers:
	clear
	@echo "Precompiling headers..."
	$(COMPILER) $(CPPFLAGS) $(TYPESBUILDARGS_GCH)
	$(COMPILER) $(CPPFLAGS) $(SABOTEUROBSERVERBUILDARGS_GCH)
	$(COMPILER) $(CPPFLAGS) $(SABOTEURBUILDARGS_GCH)
	$(COMPILER) $(CPPFLAGS) $(NAMESPACEBUILDARGS_GCH)

objects:
	clear
	@echo "Compiling Modules..."
	$(COMPILER) $(CPPFLAGS) $(SABOTEURBUILDARGS_OBJ)

saboteur:
	clear
	@echo "Assembling Saboteur Lifecycle..."
	yasm -g dwarf2 -f elf64 src/assembly/x86_64/64_bit/linux/Saboteur.asm -l ./Lifecycle.lst -o Lifecycle.o

run:
	clear
	@echo "Running..."
	./$(BIN_DIR)/$(TARGET)

debug:
	clear
	@echo "Debug Run..."
	gdb ./$(BIN_DIR)/$(TARGET)

valgrindrun:
	clear
	@echo "Running with valgrind..."
	valgrind ./$(BIN_DIR)/$(TARGET)

test:
	clear
	@echo "Compiling"
	$(COMPILER) $(CPPFLAGS) $(RUNTIMEINCLUDEPATH) $(TESTINCLUDEPATH) -o $(BIN_DIR)/$(TARGETTEST) $(RUNTIMESOURCEPATH)$(ALLCPPCONST) $(UTILITIESSOURCEPATH)$(ALLCPPCONST) $(TESTSOURCEPATH)$(ALLCPPCONST)

runtest:
	clear
	@echo "Running..."
	./$(BIN_DIR)/$(TARGETTEST)

testdebug:
	clear
	@echo "Debug Run..."
	gdb ./$(BIN_DIR)/$(TARGETTEST)

testvalgrindrun:
	clear
	@echo "Running with valgrind..."
	valgrind ./$(BIN_DIR)/$(TARGETTEST)

clean:
	clear
	@echo "Cleaning"
ifeq (,$(wildcard $(BIN_DIR)/$(TARGET).dSYM))
	rm -rf $(BIN_DIR)/$(TARGET).dSYM
endif
ifeq (,$(wildcard $(BIN_DIR)/$(TARGET).o))
	rm -rf $(BIN_DIR)/$(TARGET)
	rm -rf $(BIN_DIR)/$(TARGET).o
	rm -rf $(TYPES_GCH)
	rm -rf $(SABOTEUROBSERVER_GCH)
	rm -rf $(SABOTEUR_GCH)
	rm -rf $(NAMESPACE_GCH)
	rm -rf $(SABOTEUR_OBJ)
endif
