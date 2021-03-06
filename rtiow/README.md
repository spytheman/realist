# Ray tracing in one weekend
See here : https://raytracing.github.io
This repo contains my experiments following the excellent book above,
in C, C++, Go, Odin, Rust and V.
NOTE: current V (AST) is buggy; only v0.1.25 is know to work.

# rtiow
Benchmark :
-------------
`$ make mrproper bench`

```
/usr/bin/time C/main14.elf 1024 768 10 main14.ppm
50.56user 0.00system 0:50.63elapsed 99%CPU (0avgtext+0avgdata 4320maxresident)k
/usr/bin/time CPP/main14.elf 1024 768 10 main14.ppm
59.41user 0.09system 0:59.76elapsed 99%CPU (0avgtext+0avgdata 5912maxresident)k
/usr/bin/time Go/main14.elf 1024 768 10 main14.ppm
331.14user 1.12system 5:35.48elapsed 99%CPU (0avgtext+0avgdata 14168maxresident)k
/usr/bin/time Odin/main14.elf 1024 768 10 main14.ppm
79.13user 0.01system 1:19.49elapsed 99%CPU (0avgtext+0avgdata 4160maxresident)k
/usr/bin/time Rust/main14.elf 1024 768 10 main14.ppm
37.40user 0.09system 0:37.93elapsed 98%CPU (0avgtext+0avgdata 4668maxresident)k
/usr/bin/time V/main14.elf 1024 768 10 main14.ppm
84.65user 0.11system 1:25.66elapsed 98%CPU (0avgtext+0avgdata 4364maxresident)k
```

Conclusion (old) :
-------------------
- V version is slower than C++ (+89%, x1.85)
- This is both suspect and frustrating

```
$ /usr/bin/time V/A14 > A14.ppm
26.43user 0.00system 0:26.46elapsed 99%CPU (0avgtext+0avgdata 3108maxresident)k
0inputs+488outputs (0major+323minor)pagefaults 0swaps
$ /usr/bin/time CPP/main14 > main14.ppm
13.98user 0.00system 0:13.99elapsed 99%CPU (0avgtext+0avgdata 3904maxresident)k
0inputs+528outputs (0major+149minor)pagefaults 0swaps
```

Notes :
2/output an image
-output text PPM image RGB888
-graphics hello world : R:0=>1 LTR, G:0=>1 BTT; G=>Y, B=>R

3/the vec3 class
-vec3 class : color,location,direction,offset,...
-operators : `+,-,[],&[], +=,-=,*=,/=,*=f,/=f`

TODO :
-understand why V version is slower and fix it

# rttnw vs. rttroyl
Tests revealed that rttnw (brute force) produce more noisy result
that rttroyl (stochastic)

rttnw   500 500 1000 => 7min lot of noise
rttroyl 500 500 1000 => 8min less noise

rttnw   500 500 2000 => 15min still more noise
