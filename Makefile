.PHONY: all clean rebuild benchmark correctness profiling

all: benchmark correctness profiling

benchmark:
	mkdir -p build
	cd build && cmake .. && cmake --build . --target benchmark

correctness:
	mkdir -p build
	cd build && cmake .. && cmake --build . --target correctness

profiling:
	mkdir -p build
	cd build && cmake .. && cmake --build . --target profiling

clean:
	rm -rf build

rebuild: clean all
