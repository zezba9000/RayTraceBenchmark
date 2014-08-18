Default Compile
---
`$ nimrod c -d:release program`

Optional Defines
---
- **noSave**: Don't save the image
- **avgRun**: Run 20 times for more accurate results
- **bigImg**: Render 8x 720p resolution (10240x5760)  on single thread (for multi-thread comparison)
- **bigImgMT**: Render 8x 720p resoution multi-threaded using new ThreadPool module & 64 jobs. Note: requires `--threads:on`