COMPILER:=g++
CPPFLAGS:=-Wall -Wextra -g -pedantic  -std=c++17 -pthread
TARGET:=processjruntime
TARGETTEST:=processjruntimetest

# ----------
# Extensions

ALLHPPCONST:=*.hpp
ALLCPPCONST:=*.cpp

# -----------
# Directories

BIN:=bin
INCLUDE:=include
SOURCE:=src
TEST:=test

# -----
# Paths

INCLUDEPATH:=-I$(INCLUDE)/
SOURCEPATH:=$(SOURCE)/

# -------------
# Include Paths

TESTINCLUDEPATH:=$(INCLUDEPATH)$(TEST)/
RUNTIMEINCLUDEPATH:=$(INCLUDEPATH)

# -----------
# Source Path

# -------
# Targets

$(BIN)/$(TARGET):
	clear
	@echo "🚧 Compiling"
	$(COMPILER) $(CPPFLAGS) $(RUNTIMEINCLUDEPATH) -o $(BIN)/$(TARGET) $(RUNTIMESOURCEPATH)$(ALLCPPCONST)

run:
	clear
	@echo "🚀 Running..."
	./$(BIN)/$(TARGET)

debug:
	clear
	@echo "🚫🐛 Debug Run..."
	gdb ./$(BIN)/$(TARGET)

valgrindrun:
	clear
	@echo "🚀 Running with valgrind..."
	valgrind ./$(BIN)/$(TARGET)

test:
	clear
	@echo "🚧 Compiling"
	$(COMPILER) $(CPPFLAGS) $(RUNTIMEINCLUDEPATH) $(TESTINCLUDEPATH) -o $(BIN)/$(TARGETTEST) $(RUNTIMESOURCEPATH)$(ALLCPPCONST) $(UTILITIESSOURCEPATH)$(ALLCPPCONST) $(TESTSOURCEPATH)$(ALLCPPCONST)

runtest:
	clear
	@echo "🚀 Running..."
	./$(BIN)/$(TARGETTEST)

testdebug:
	clear
	@echo "🚫🐛 Debug Run..."
	gdb ./$(BIN)/$(TARGETTEST)

testvalgrindrun:
	clear
	@echo "🚀 Running with valgrind..."
	valgrind ./$(BIN)/$(TARGETTEST)

clean:
	clear
	@echo "🧹💩 Cleaning"
ifeq (,$(wildcard $(BIN)/$(TARGET).dSYM))
	rm -rf $(BIN)/$(TARGET).dSYM
endif
ifeq (,$(wildcard $(BIN)/$(TARGET).o))
	rm -rf $(BIN)/$(TARGET)
	rm -rf $(BIN)/$(TARGET).o
endif

cleantest:
	clear
	@echo "🧹💩 Cleaning"
ifeq (,$(wildcard $(BIN)/$(TARGETTEST).dSYM))
	rm -rf $(BIN)/$(TARGETTEST).dSYM
endif
ifeq (,$(wildcard $(BIN)/$(TARGETTEST).o))
	rm -rf $(BIN)/$(TARGETTEST)
	rm -rf $(BIN)/$(TARGETTEST).o
endif
