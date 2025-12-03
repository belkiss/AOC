CC=clang++
CPPFLAGS=-std=c++20 -Wall -Wextra -Werror

day1: day1/day1.cpp
	$(CC) -c $(CPPFLAGS) $<

# %.*: %.cpp
# 	$(CC) -c $(CPPFLAGS) $< -o $@

# day*: $(wildcard $@/*.cpp)
# 	$(CC) -o $@ $<

clean:
	rm -rvf day*/day

# print: $(wildcard day*/*.cpp)
# 	ls -l $?