# Video compression assignment  

Amount of mandays worked: 14  
Division of tasks: pair programming  
Status project: Finished, still testing on different files  

## Details  
 - AC and DC components use different amount of bits in every block  
 - Motioncompensation is calculated on the decoded previous frame for more accuracy  
 
## Implemented features  

 - I-frames encode and decode  
 - Calculate motion vectors  
 - Create P-frames and enhance with difference  
 - Decode P-frames  
 - GOP changeable  
 - Motioncompensation setting  

==> All required features are implemented.  

## TODO / still need to implement  
 - Searching for artefacts / bugs  
 
## Noticeable artefacts  
 - None  

## Results  
### Result 1  
input: samples/SOCCER_352x288_30_orig_02.yuv -> 44550 kB  
encoded (with RLE): SOCCER_352x288_30_orig_02.enc -> 6375 kB  
encoded (without RLE): SOCCER_352x288_30_orig_02.enc -> 10737 kB  
decoded: SOCCER_352x288_30_orig_02_result.yuv -> 44550 kB  

compression ratio (with RLE):  0.143097643  
compression ratio (without RLE):  0.241010101  

with quantization matrix:  
8 8 16 32  
8 12 32 64  
16 32 64 128  
32 64 128 256  

compress time (with RLE): 40.887s  
decompress time (with RLE): 2.723s  

compress time (without RLE): 40.481s  
decompress time (without RLE): 2.271s  

### Result 2  
input: samples/HARBOUR_352x288_30_orig_01.yuv -> 44550 kB  
encoded (with RLE): HARBOUR_352x288_30_orig_01.enc -> 8063 kB  
decoded: HARBOUR_352x288_30_orig_01_result.yuv -> 44550 kB  

compression ratio (with RLE):  0.180987654  

with quantization matrix:  
8 8 16 32  
8 12 32 64  
16 32 64 128  
32 64 128 256  

compress time (with RLE): 43.552s  
decompress time (with RLE): 2.778s  