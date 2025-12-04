CXX := clang++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -Wno-unused-function

# Colors
GREEN := \033[1;32m
BLUE := \033[1;34m
RESET := \033[0m

# All day directories (e.g. 2024/day1, 2025/day3)
DAY_DIRS := $(wildcard */day*)

# Find all .cpp files inside year/day folders
SOURCES := $(wildcard */day*/day*.cpp)

# Convert each .cpp (e.g. 2024/day1/day1.cpp) into a binary path (2024/day1/day1)
BINARIES := $(SOURCES:.cpp=)

# Pattern rule: build any day binary from its matching .cpp
%/day%/day% : ; # dummy to avoid some Make quirks (not strictly required)
% : %.cpp
	@echo -e "$(GREEN)[CXX]$(RESET) Building $@"
	@$(CXX) $(CXXFLAGS) -o $@ $<

# A more general and reliable pattern:
# when target is X/Y (binary path) and source is X/Y.cpp, previous rule covers it because of .cpp suffix.
# But to be explicit:
%/day%/%: %/day%/%.cpp
	@printf "$(GREEN)[CXX]$(RESET) Building %s\n" "$@"
	@$(CXX) $(CXXFLAGS) -o $@ $<

# Year targets: build all binaries in that year
# Example: running `make 2024` builds all 2024/day*/day*
YEARS := $(sort $(dir $(wildcard */)))

# Remove trailing slash for nicer targets
YEAR_TARGETS := $(patsubst %/,%,$(YEARS))

.PHONY: all clean $(YEAR_TARGETS) $(DAY_DIRS)

all: $(BINARIES)

# Map directory-style targets (e.g. "2024/day1") to the real binary ("2024/day1/day1")
# We mark day directories as phony above, so make will run this recipe even though the directory exists
$(DAY_DIRS):
	@printf "$(BLUE)[DIR]$(RESET) Building %s\n" "$@"
	@$(MAKE) $@/$(@F)

# Build all days inside a year
$(YEAR_TARGETS):
	@echo -e "$(BLUE)[YEAR]$(RESET) Building all in $@"
	@$(MAKE) $(filter $@/%,$(BINARIES))

#day1: day1/day1.cpp
#	$(CC) -c $(CPPFLAGS) $<

# %.*: %.cpp
# 	$(CC) -c $(CPPFLAGS) $< -o $@

# day*: $(wildcard $@/*.cpp)
# 	$(CC) -o $@ $<

clean:
	@echo -e "$(BLUE)[CLEAN]$(RESET) Removing all built binaries"
	@rm -f $(BINARIES)

# print: $(wildcard day*/*.cpp)
# 	ls -l $?
