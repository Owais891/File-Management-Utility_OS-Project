# Makefile for the File Management Utility
# Operating Systems Semester Project (C++17 / Linux / POSIX)
#
# Usage:
#   make            Build the executable (./file_manager)
#   make run        Build and run the executable
#   make clean      Remove all generated files
#   make            # then ./file_manager
#
# Manual one-line compilation (if make is unavailable):
#   g++ -std=c++17 -Wall -Wextra -O2 -o file_manager \
#       main.cpp file_manager.cpp directory_manager.cpp file_search.cpp \
#       metadata.cpp permissions.cpp file_organizer.cpp duplicate_finder.cpp \
#       activity_log.cpp utils.cpp

# Compiler
CXX        := g++
CXXFLAGS   := -std=c++17 -Wall -Wextra -O2
LDFLAGS    :=

# Target binary name
TARGET     := file_manager

# Object files (one per .cpp)
OBJS := \
    main.o \
    file_manager.o \
    directory_manager.o \
    file_search.o \
    metadata.o \
    permissions.o \
    file_organizer.o \
    duplicate_finder.o \
    activity_log.o \
    utils.o

# Default goal
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Pattern rule: compile each .cpp into a .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Header dependencies: rebuild every object if any header changes
$(OBJS): utils.h file_manager.h directory_manager.h file_search.h \
         metadata.h permissions.h file_organizer.h duplicate_finder.h \
         activity_log.h

# Convenience target
run: $(TARGET)
	./$(TARGET)

# Clean generated files
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Cleaned object files and executable."

.PHONY: all clean run
