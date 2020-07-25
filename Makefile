# Use clang as the C compiler
CC = clang
# Flags to pass to clang:
# -Iinclude tells clang to look for #include files in the "include" folder
# -Wall turns on all warnings
# -g adds filenames and line numbers to the executable for useful stack traces
# -fno-omit-frame-pointer allows stack traces to be generated
#   (take CS 24 for a full explanation)
# -fsanitize=address enables asan
CFLAGS = -Iinclude -Wall -g -fno-omit-frame-pointer -fsanitize=address

# Compiler flag that links the program with the math library
LIB_MATH = -lm
# Compiler flags that link the program with the math and SDL libraries.
# Note that $(...) substitutes a variable's value, so this line is equivalent to
# LIBS = -lm -lSDL2 -lSDL2_gfx
LIBS = $(LIB_MATH) -lSDL2 -lSDL2_gfx -lSDL2_ttf -lSDL2_mixer 

# List of demo programs
DEMOS = game 
# List of C files in "libraries" that we provide
STAFF_LIBS = test_util sdl_wrapper
# List of C files in "libraries" that you will write
STUDENT_LIBS = vector list \
	body scene collision \
	polygon forces sdnoise1234 

STUDENT_TESTS = $(subst .c,, $(subst tests/student/,,$(wildcard tests/student/*.c)))

# List of compiled .o files corresponding to STUDENT_LIBS, e.g. "out/vector.o".
# Don't worry about the syntax; it's just adding "out/" to the start
# and ".o" to the end of each value in STUDENT_LIBS.
STUDENT_OBJS = $(addprefix out/,$(STUDENT_LIBS:=.o))
# List of test suite executables, e.g. "bin/test_suite_vector"
TEST_BINS = $(addprefix bin/test_suite_,$(STUDENT_LIBS)) $(addprefix bin/,$(STUDENT_TESTS))

# List of demo executables, i.e. "bin/bounce".
DEMO_BINS = $(addprefix bin/,$(DEMOS))
# All executables (the concatenation of TEST_BINS and DEMO_BINS)
BINS = $(TEST_BINS) $(DEMO_BINS)

# The first Make rule. It is relatively simple:
# "To build 'all', make sure all files in BINS are up to date."
# You can execute this rule by running the command "make all", or just "make".
all: $(BINS)

# Any .o file in "out" is built from the corresponding C file.
# Although .c files can be directly compiled into an executable, first building
# .o files reduces the amount of work needed to rebuild the executable.
# For example, if only vector.c was modified since the last build, only vector.o
# gets recompiled, and clang reuses the other .o files to build the executable.
#
# "%" means "any string".
# Unlike "all", this target has a build command.
# "$^" is a special variable meaning "the source files"
# and $@ means "the target file", so the command tells clang
# to compile the source C file into the target .o file.
out/%.o: library/%.c # source file may be found in "library"
	$(CC) -c $(CFLAGS) $^ -o $@
out/%.o: tests/%.c # or "tests"
	$(CC) -c $(CFLAGS) $^ -o $@
out/%.o: tests/student/%.c # or "tests"
	$(CC) -c $(CFLAGS) $^ -o $@

out/demo-%.o: demo/%.c # or "demo"; in this case, add "demo-" to the .o filename
	$(CC) -c $(CFLAGS) $^ -o $@

# Builds the demos by linking the necessary .o files.
# Unlike the out/%.o rule, this uses the LIBS flags and omits the -c flag,
# since it is building a full executable.
bin/%: out/demo-%.o out/sdl_wrapper.o $(STUDENT_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

# Builds the test suite executables from the corresponding test .o file
# and the library .o files. The only difference from the demo build command
# is that it doesn't link the SDL libraries.
bin/test_suite_%: out/test_suite_%.o out/test_util.o $(STUDENT_OBJS)
	$(CC) $(CFLAGS) $(LIB_MATH) $^ -o $@

# Builds your test suite executable from your test .o file and the library
# files. Once again we don't link SDL, so your test cannot use SDL either.
bin/student_tests: out/student_tests.o out/test_util.o $(STUDENT_OBJS)
	$(CC) $(CFLAGS) $(LIB_MATH) $^ -o $@

bin/%_tests: out/%_tests.o out/test_util.o $(STUDENT_OBJS)
	$(CC) $(CFLAGS) $(LIB_MATH) $^ -o $@


# Runs the tests. "$(TEST_BINS)" requires the test executables to be up to date.
# The command is a simple shell script:
# "set -e" configures the shell to exit if any of the tests fail
# "for f in $(TEST_BINS); do ...; done" loops over the test executables,
#    assigning the variable f to each one
# "$$f" runs the test; "$$" escapes the $ character,
#   and "$f" tells the shell to substitute the value of the variable f
# "echo" prints a newline after each test's output, for readability
test: $(TEST_BINS)
	set -e; for f in $(TEST_BINS); do $$f; echo; done

# Removes all compiled files. "out/*" matches all files in the "out" directory
# and "bin/*" does the same for the "bin" directory.
# "rm" deletes the files; "-f" means "succeed even if no files were removed".
# Note that this target has no sources, which is perfectly valid.
clean:
	rm -f out/* bin/*

# This special rule tells Make that "all", "clean", and "test" are rules
# that don't build a file.
.PHONY: all clean test
# Tells Make not to delete the .o files after the executable is built
.PRECIOUS: out/%.o out/demo-%.o
