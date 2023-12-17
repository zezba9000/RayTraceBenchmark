mkdir bin
g++ -O3 -flto -march=native -mtune=native -DWIN32_GCC ../Common/RayTraceBenchmark.cpp -o ./bin/RayTraceBenchmark