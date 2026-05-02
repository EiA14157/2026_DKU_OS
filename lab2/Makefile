# Compiler 변수 설정: g++ 사용
CXX=g++

# 컴파일러 플래그 설정: C++11 표준 사용
CXXFLAGS=-std=c++14 -g

LDFLAGS=-lpthread -lgtest

# Source files
SOURCES=test.cpp hashtable_impl.cpp test_util.cpp
OBJECTS=$(SOURCES:.cpp=.o)

# Executable name
EXECUTABLE=test

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
