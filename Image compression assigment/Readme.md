# Image compression assignment

Man-days worked: 6 days  
Division of tasks: pair programming  

## Implemented features

Subdivision in 4x4 blocks,  
Block transformation using DCT,  
Quantization of transformed blocks,  
(logging of DCT and Quantization steps,)  
Storing quantized coefficients in a list using zig-zag scan,  
Optional run-length encoding (RLE) encoding  

## Non-implemented features

Huffman coding, arithmetic coding  

## Noticeable artefacts

White points becoming black, because of an 8-bit overflow.  
--> This was solved.  

Black points becoming white, because of an 8-bit underflow.  
--> This was solved.  

## Roundings
After quantization, numbers are rounded down removing all decimal numbers before storing.  

## Results
# Result 1
input: image_1200x900.raw -> 1055kB  
encoded (with RLE): image_1200x900.enc -> 274kB  
decoded: image_1200x900_result.raw -> 1055kB  

compression ratio (with RLE):  0.259715640  

with quantization matrix:  
10 10 15 20  
10 15 20 25  
15 20 25 30  
20 25 30 35  


# Result 2
input: image_1024x1024.raw -> 1,048,576 bytes  
encoded (with RLE): image_1024x1024.enc -> 354,486 bytes  
encoded (without RLE): image_1024x1024.enc -> 1,048,653 bytes  
decoded: image_1024_1024_result.raw -> 1,048,576 bytes  

compression ratio (with RLE):  0.338064194  
compression ratio (without RLE):  1.000073433  

with quantization matrix:  
10 10 15 20  
10 15 20 25  
15 20 25 30  
20 25 30 35  

bits per DC or AC component (with RLE): 11 (for every block)  
bits per DC or AC component (without RLE): 8 (for every block)  

compress time (with RLE): 1.283s  
decompress time (with RLE): 0.128s  

# Result 2
input: image_1024x1024.raw -> 1,048,576 bytes  
encoded (with RLE): image_1024x1024.enc -> 228,232 bytes
encoded (without RLE): image_1024x1024.enc -> 917,581 bytes
decoded: image_1024_1024_result.raw -> 1,048,576 bytes  

compression ratio (with RLE): 0.217658997  
compression ratio (without RLE): 0.875073433  

with quantization matrix:  
20 20 30 40  
20 30 40 50  
30 40 50 60  
40 50 60 70  

bits per DC or AC component (with RLE): 10 (for every block)  
bits per DC or AC component (without RLE): 7 (for every block)  

compress time (with RLE): 1.275s  
decompress time (with RLE): 0.119s  

compress time (without RLE): 1.305s  
decompress time (without RLE): 0.124s  
