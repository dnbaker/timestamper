.PHONY: all clean

ts: ts.cpp timestamper.h
	$(CXX) -O3 $< -o $@

all: ts

clean:
	rm ts
