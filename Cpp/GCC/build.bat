mkdir bin
g++ -std=c++11 -O3 -flto -DWIN32_GCC ../Common/RayTraceBenchmark.cpp -o ./bin/RayTraceBenchmark