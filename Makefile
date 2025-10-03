# Minimal, high-safety Makefile (Linux)
# Targets: debug (ASan+UBSan), tsan (ThreadSanitizer), release (hardened)

.SILENT:
.PHONY: all debug tsan release run-debug run-tsan run-release clean

# --- config ---
CXX      ?= g++
SRC       = task-cli.cpp
BIN       = task-cli

WARNINGS = -Wall -Wextra -Wpedantic -Werror \
           -Wconversion -Wsign-conversion -Wshadow \
           -Wformat=2 -Wnull-dereference -Wdouble-promotion \
           -Wimplicit-fallthrough -Wundef -Wnon-virtual-dtor

# Generate dep files for correct incremental rebuilds
DEPFLAGS = -MMD -MP

# Common link flags (empty for now; set per target)
LDFLAGS  =

all: debug

# --- Debug: ASan + UBSan (fast, catches most memory/UB) ---
debug: CXXFLAGS = -O1 -g $(WARNINGS) $(DEPFLAGS) \
                  -fsanitize=address,undefined -fno-omit-frame-pointer \
                  -D_GLIBCXX_ASSERTIONS
debug: LDFLAGS  = -fsanitize=address,undefined
debug: $(BIN)-debug

$(BIN)-debug: $(SRC:.cpp=.debug.o)
	$(CXX) $^ -o $@ $(LDFLAGS)

%.debug.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run-debug: debug
	ASAN_OPTIONS=halt_on_error=1:strict_string_checks=1:detect_stack_use_after_return=1 \
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
	./$(BIN)-debug

# --- ThreadSanitizer build (separate job; can’t mix with ASan) ---
tsan: CXXFLAGS = -O1 -g $(WARNINGS) $(DEPFLAGS) \
                 -fsanitize=thread -fno-omit-frame-pointer \
                 -D_GLIBCXX_ASSERTIONS
tsan: LDFLAGS  = -fsanitize=thread
tsan: $(BIN)-tsan

$(BIN)-tsan: $(SRC:.cpp=.tsan.o)
	$(CXX) $^ -o $@ $(LDFLAGS)

%.tsan.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run-tsan: tsan
	TSAN_OPTIONS=halt_on_error=1 ./$(BIN)-tsan

# --- Hardened release (mitigations on; no sanitizers) ---
release: CXXFLAGS = -O2 -DNDEBUG $(WARNINGS) $(DEPFLAGS) \
                    -D_FORTIFY_SOURCE=3 -fstack-protector-strong \
                    -fstack-clash-protection -fPIE
release: LDFLAGS  = -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack
release: $(BIN)

$(BIN): $(SRC:.cpp=.rel.o)
	$(CXX) $^ -o $@ $(LDFLAGS)

%.rel.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run-release: release
	./$(BIN)

clean:
	rm -f $(BIN) $(BIN)-debug $(BIN)-tsan \
	      *.debug.o *.tsan.o *.rel.o *.d

# Include dep files if present
-include *.d
