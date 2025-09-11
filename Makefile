CXX := cl
CXXFLAGS := /EHsc /W4 /std:c++17
LDFLAGS := dwmapi.lib user32.lib

SRC := src\main.cpp
TARGET := win-workspaces.exe

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) /Fe$@ /link $(LDFLAGS)

clean:
	del $(TARGET) *.obj
