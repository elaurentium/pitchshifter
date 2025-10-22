run:
	@if [ "$(shell uname)" == "Linux" ]; then \
		pulseaudio --start; \
	fi
	./build/bin/PitchShifter

build:
	chmod +x build.sh
	./build.sh

clean:
	rm -fr build