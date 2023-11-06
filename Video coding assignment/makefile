all: build/encoder build/decoder

run:
	./build/encoder encsettings.conf
	./build/decoder decsettings.conf

build_run_encoder: build/encoder
	./build/encoder encsettings.conf

build_run_decoder: build/decoder
	./build/decoder decsettings.conf

build/encoder: code/encoder/*.cpp code/encoder/*.h code/decoder/*.h code/decoder/image_decoder.cpp code/util/*.h code/util/*.cpp
	g++ -O3 -o $@ $^ -Icode/util

build/decoder: code/decoder/*.cpp code/decoder/*.h code/util/*.h code/util/*.cpp
	g++ -O3 -o $@ $^ -Icode/util
