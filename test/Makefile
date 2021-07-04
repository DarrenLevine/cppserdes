all: test
.PHONY : all

# prints a help page
define help_page
"\
The following are the valid targets for this Makefile:               \n \
... all       : runs "test" (the default if no target is provided)   \n \
... test      : runs all unit tests                                  \n \
... docs      : generates doxygen documentation and opens it         \n \
... clean     : removes build, doc, and gcov files                   \n \
... test_gcov : runs test with coverage reports                      \n \
... msys32    : runs the tests with 32bit mingw if installed         \n \
... clang     : runs the tests with clang if installed               \n"
endef
help:
	@echo -e $(help_page)
.PHONY : help

CPP_STANDARD = -std=c++11
CXX ?= g++
doxygen_output = ../docs/html/index.html

LOTS_OF_WARNINGS = \
	-Werror -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn \
	-Wsuggest-attribute=format -Wmissing-format-attribute -Wall -Wextra -pedantic -Wcast-align -Wcast-qual \
	-Wctor-dtor-privacy -Wdisabled-optimization -Winit-self -Wlogical-op -Wmissing-declarations \
	-Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo \
	-Wstrict-null-sentinel -Wswitch-default -Wundef -Wno-unused -Wdouble-promotion -Wstrict-overflow=5 \
	-Wstrict-aliasing=2 -Wformat=2 -Weffc++
ifneq ($(MAKECMDGOALS),$(clang))
CXX := clang++
LOTS_OF_WARNINGS := \
	-Werror -Wmissing-format-attribute -Wall -Wextra -pedantic -Wcast-align -Wcast-qual \
	-Wctor-dtor-privacy -Wdisabled-optimization -Winit-self -Wmissing-declarations \
	-Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo \
	-Wswitch-default -Wundef -Wno-unused -Wdouble-promotion -Wstrict-overflow=5 \
	-Wstrict-aliasing=2 -Wformat=2 -Weffc++
endif
src = test_all.cpp
srcs = $(src) test_multiple_cpp_files.cpp

ifeq ($(OS),Windows_NT)
prog_name = $(basename $(src)).exe
else
prog_name = $(basename $(src)).elf
endif

# runs all unit tests
test:
	@echo "compiling ..." && \
	$(CXX) $(srcs) $(CPP_STANDARD) -O3 $(LOTS_OF_WARNINGS) -o $(prog_name) && \
	echo "running ..." && \
	./$(prog_name) || exit 1 && \
	rm -f $(prog_name)
.PHONY : test

# runs all unit tests using /c/msys64/mingw32/bin/g++.exe 32bit compiler 
msys32: test
.PHONY: msys32
ifneq ($(MAKECMDGOALS),$(msys32))
export PATH := /c/msys64/mingw32/bin/:$(PATH)
endif

# runs all unit tests using /c/msys64/mingw32/bin/g++.exe 32bit compiler 
clang: test
.PHONY: clang

# runs all unit tests and generates gcov coverage reports
test_gcov:
	@echo "compiling ..." && \
	$(CXX) $(srcs) $(CPP_STANDARD) -O0 --coverage -fno-inline -fno-inline-small-functions -fno-default-inline -fprofile-arcs -ftest-coverage $(LOTS_OF_WARNINGS) -o $(prog_name) && \
	echo "running ..." && \
	./$(prog_name) && \
	echo "" && \
	gcov $(srcs) && \
	rm -f $(prog_name) *.gcda *.gcno
.PHONY : test_gcov

# opens documentation (and generates documentation if they are out of date)
docs: $(doxygen_output)
	$(open_html_commnad) $(doxygen_output)
.PHONY : docs
ifeq ($(OS),Windows_NT)
open_html_commnad = start
else
open_html_commnad = open
endif

# removes all build, gcov, and docs files
clean:
	rm -f $(prog_name) *.gcda *.gcno *.gcov && \
	cd ../ && \
	rm -r docs
.PHONY : clean

doxygen_dependencies = \
	$(wildcard ../include/*) \
	$(wildcard ../examples/*) \
	$(wildcard ../images/*) \
	../README.md ../Doxyfile

$(doxygen_output): $(doxygen_dependencies)
	cd ../ && doxygen Doxyfile
