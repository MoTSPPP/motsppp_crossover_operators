CXX      = g++
CXXFLAGS = -std=c++17 -O3 -g

SRCS = main.cpp \
       moead/Grid.cpp \
       moead/ParetoSet.cpp \
       moead/BoundedParetoSet.cpp \
       utils/solution_utils.cpp \
       utils/io_utils.cpp \
       utils/population_utils.cpp \
       utils/generic_algorithms.cpp \
       moead/moead.cpp

# operators/{mutation,crossover}.cpp NAO estao na lista pois sao incluidos via
# #include de population_utils.cpp e moead.cpp

ifeq ($(OS),Windows_NT)
    TARGET = main.exe
    RM = del /Q
else
    UNAME_S := $(shell uname -s)
    TARGET = main
    RM = rm -f
endif

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	$(RM) $(TARGET)

.PHONY: all clean
