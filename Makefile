CXX = g++
CXXFLAGS = -std=c++17 -I./vcpkg/installed/x64-windows/include
LDFLAGS = -L./vcpkg/installed/x64-windows/lib -lgtest -lgtest_main -pthread

SRC = $(wildcard tests/test_src/src/**/*.cc)
OUT = bin/test_exec

all: $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $(OUT) $(LDFLAGS)

clean:
	del /f /q bin\test_exec.exe
