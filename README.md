# ImageCrop
A simple Windows command-line tool for cropping multi-monitor **PNG** "PrtScrn" images down to a single monitor

## Getting Started

### Prerequisites
* C++17 flag is required for compilation (std::filesystem)
* The OpenCV library is needed [OpenCV](https://github.com/opencv/opencv)
* The C++11 port of docopt is used for command-line argument parsing [docopt](https://github.com/docopt/docopt.cpp)
* spdlog is used for logging [spdlog](https://github.com/gabime/spdlog)
* Platform specific (Windows)

### Current Features
* Crop all multi-monitor **PNG** Images in the specified directory dynamically based on the given monitor ID.
* -r for recursive directory search.

### In Progress

### Current Limitations
* Other Image types (instead of using opencv to load each image and check its size, a custom header reader **ONLY** for PNGs 
<br/>is used to read just enough bytes to discern the size).

## Authors

* **Dimitrios Kazakos** - *Initial work* - [Gast91](https://github.com/Gast91)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
<br/>The *unmodified* OpenCV library is used under the Apache 2.0 licence - see the [LICENSE.md](https://github.com/opencv/opencv/blob/master/LICENSE) for details
<br/>docopt is used under the Boost Software Licence - Version 1.0 - see the [LICENSE.md](https://github.com/docopt/docopt.cpp/blob/master/LICENSE-Boost-1.0) for details