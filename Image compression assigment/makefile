
all: build/encoder build/decoder

run:
	./build/encoder settings.conf
	./build/decoder settings.conf

build_run_encoder: build/encoder
	./build/encoder settings.conf

build_run_decoder: build/decoder
	./build/decoder settings.conf

build/encoder: code/encoder/main.cpp code/encoder/encoder.cpp code/util/BitStream.cpp code/encoder/encoder.h code/util/*.h code/util/*.cpp
	g++ -O3 -o $@ $^ -Icode/util

build/decoder: code/decoder/main.cpp code/decoder/decoder.cpp code/util/BitStream.cpp code/decoder/decoder.h code/util/*.h code/util/*.cpp
	g++ -O3 -o $@ $^ -Icode/util
