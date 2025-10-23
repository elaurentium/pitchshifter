INCLUDE=src/core/config.h

all: build run

run:
	@if [ "$(shell uname)" == "Linux" ]; then \
		pulseaudio --start; \
	fi
	./build/bin/PitchShifter

build:
	chmod +x build.sh
	./build.sh

clean:
	rm -fr $(INCLUDE)
	rm -fr build