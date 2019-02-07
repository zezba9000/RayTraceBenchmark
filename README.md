RayTraceBenchmark
=================

A benchmark for testing the performance of different languages while rendering a ray-traced world.

Code originally ported from: http://forum.dlang.org/thread/yzsqwejxqlnzryhrkfuq@forum.dlang.org#post-yzsqwejxqlnzryhrkfuq:40forum.dlang.org


All benchmarks will produce a RAW image that is equivelent to ReferenceImage.jpg

The RAW image resolution is 1280x720.
Image can be converted to png with ImageMajic:<br>
"magick.exe convert -size 1280X720 -depth 8 <PathTo>/Image.rgb <PathTo>/Image.png"

C# Results:
---
https://github.com/zezba9000/RayTraceBenchmark/blob/master/C%23/Results.md

Cpp Results:
---
https://github.com/zezba9000/RayTraceBenchmark/blob/master/Cpp/Results.md

Nim Results:
---
https://github.com/zezba9000/RayTraceBenchmark/blob/master/Nim/Results.md
