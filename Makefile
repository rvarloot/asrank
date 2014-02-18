CXX=g++
CXXFLAGS=-c -Wall -Wextra -O2# -g -pg
LDFLAGS=#-g -pg
EXEC=asrank
SRC=main.cpp io.cpp inference.cpp data.cpp
OBJ=$(SRC:.cpp=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(LDFLAGS) $^ -o $(EXEC)

main.o: io.h inference.h data.h
data.o: data.h io.h
io.o: io.h data.h
inference.o: inference.h data.h

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY: clean mrproper

clean:
	rm -rf *o

mrproper: clean
	rm -rf $(EXEC)

