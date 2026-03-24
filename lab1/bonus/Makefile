# Compiler 변수 설정: g++ 사용
CXX = g++

# 컴파일러 플래그 설정
CXXFLAGS = -std=c++14 -g -Wno-deprecated-declarations

# 라이브러리 플래그
LDFLAGS = -lpthread -lgtest -lcrypto 

# Source files
SOURCES = test.cpp sched.cpp test_util.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Executable names
TEST_EXECUTABLE = test

# test 실행 파일 생성
$(TEST_EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

# 개별 .cpp 파일을 .o 오브젝트 파일로 변환
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TEST_EXECUTABLE)
