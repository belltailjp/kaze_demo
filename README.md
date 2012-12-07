# KAZE local feature matcher demo

A simple example of image matcher using KAZE local feature which is published in ECCV2012.

_Pablo F. Alcantarilla, Adrien Bartoli and Andrew J. Davison., "KAZE Features," In European Conference on Computer Vision (ECCV), Fiorenze, Italy, October 2012._
([Project Page](http://www.robesafe.com/personal/pablo.alcantarilla/kaze.html), [Implementation](https://github.com/pablofdezalc/kaze))

And this sample code is developed by @belltailjp personally for the blog article [ECCV2012で発表されたKAZE局所特徴量を試してみた | さかな前線](http://daily.belltail.jp/?p=1352#hs_9bb14cfc32bcc907176d4323f74c92d4_header_5) (Japanese).


## Requirements

This sample code is confirmed to work on the following configuration:

- Linux Mint 3.6 (Equivalent to Ubuntu 12.04)
- GCC 4.7
- OpenCV 2.4.3
- Boost C++ Library
- KAZE implementation in the same directory as the sample code

And a webcamera is needed for `demo.cpp`

## How to execute

```bash
# To build simulation matcher
% g++ simulation.cpp KAZE.cpp Ipoint.cpp nldiffusion_functions.cpp utils.cpp \
    -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_features2d -lopencv_nonfree \
    -lboost_system -lboost_thread -O3
% ./a.out

# To build the intractive matcher demo
% g++ demo.cpp KAZE.cpp Ipoint.cpp nldiffusion_functions.cpp utils.cpp \
    -lopencv_core -lopencv_highgui -lopencv_nonfree -lopencv_imgproc -lopencv_legacy -lopencv_features2d \
    -lboost_system -lboost_thread -O3
% ./a.out
```

