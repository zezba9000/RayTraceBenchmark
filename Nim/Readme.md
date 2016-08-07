Default Compile
---
`$ nim c -d:release raytrace`

8x Resolution
---
`$ nim c -d:{release,bigImg} raytrace`

8x Resolution Multi-Threaded
---
`$ nim c -d:{release,bigImgMT} --threads:on raytrace`

Optional Defines
---
- **noSave**: Don't save the image
- **avgRun**: Run 20 times for more accurate results
- **bigImg**: Render 8x 720p resolution (10240x5760)  on single thread (for multi-thread comparison)
- **bigImgMT**: Render 8x 720p resoution multi-threaded using new ThreadPool module & 64 jobs.

Checking Result
---
Benchmarking is built-in and times (in seconds) are printed to the console. To verify the rendered `image.rgb` is correct, either open with Photoshop or convert using *ImageMagick* by running `$ convert -size 1280x720 -depth 8 image.rgb image.png` and compare to the reference image.
