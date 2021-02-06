# ImageCrop
A simple Windows command-line tool for cropping multi-monitor PNG "PrtScr" images down to a single monitor

## Getting Started

### Prerequisites
* C++17 flag is required for compilation (std::filesystem)
* The OpenCV library is needed [OpenCV](https://github.com/opencv/opencv)

### Current Features
* Crop all multi-monitor PNG Images (usually taken via a "PrtScr") in the directory specified dynamically based on the given monitor ID

### In Progress
* DocOpt Integration for better commandline arguments [Docopt](https://github.com/docopt/docopt.cpp)
* Other Image types

### Current Limitations
  N/A

## Authors

* **Dimitrios Kazakos** - *Initial work* - [Gast91](https://github.com/Gast91)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
The *unmodified* OpenCV library is used under the Apache 2.0 licence - see the [LICENSE.md](https://github.com/opencv/opencv/blob/master/LICENSE) for details
