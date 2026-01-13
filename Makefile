INCLUDE=src/core/config.h

all: build run

run:
	./build/bin/PitchShifter

build:
	chmod +x build.sh
	./build.sh

clean:
	rm -fr $(INCLUDE)
	rm -fr build