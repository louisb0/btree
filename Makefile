.PHONY: all clean rebuild benchmark correctness profile

all: bench test profile

bench:
	mkdir -p build
	cd build && cmake .. && cmake --build . --target bench

test:
	mkdir -p build
	cd build && cmake .. && cmake --build . --target test 

profile:
	mkdir -p build
	cd build && cmake .. && cmake --build . --target profile

clean:
	rm -rf build

rebuild: clean all
