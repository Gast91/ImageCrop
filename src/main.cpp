#include <iostream>

#include "Monitor.h"
//#include "../vendor/docopt/docopt.h"
//
//static const char USAGE[] =
//R"(Image Crop.
//    Usage:
//      ImageCrop --path=<path/to/dir>
//      ImageCrop --path=<path/to/dir> [-m=<Num> | --monitor=<Num>]
//      ImageCrop --path=<path/to/dir> [-m=<Num> | --monitor=<Num>] [-v | --verbose] 
//      ImageCrop --path=<path/to/dir> [-m=<Num> | --monitor=<Num>] [-v | --verbose] [-r | --recursive]  
//      ImageCrop (-h | --help)
//      ImageCrop --version
//    Options:
//      -h --help        Show this screen.
//      --version        Show version.
//      -p --path        Path to directory (required).
//	  -m --monitor     Monitor to crop   (defaults to primary).
//      -v --verbose     Show verbose output.
//      -r --recursive   Crop images in the subdirectories also.
//)";

int main(int argc, char* argv[])
{
	//std::map<std::string, docopt::value> args
	//	= docopt::docopt(USAGE,
	//				    { argv + 1, argv + argc },
	//		              true,              // Show Help if requested
	//		              "ImageCrop 1.0");  // Version string
	//for (const auto& arg : args)
	//{
	//	std::cout << arg.first << arg.second << '\n';
	//}
	//std::cin.get();
#ifndef _DEBUG
	if (argc == 3) // DOCOPT!
	{
		// TODO:
		// docopt for arguments --> -verbose, -recursive, path is needed, display id optional (default primary monitor)
		// spdlog?
		// opencv as static rather than dll
		// global vector...
		// split into a monitor.h file?
		std::string path = argv[1];
		int cropDisplay = atoi(argv[2]);
		const auto displayNum = GetSystemMetrics(SM_CMONITORS);
		if (cropDisplay < 1 || cropDisplay > displayNum) { std::cout << "Invalid display ID\n"; return -1; }
#else
	std::string path = R"(D:\Desktop\test)";
	const unsigned int cropDisplay = 1;
#endif

	if (!GetAllMonitorInfo()) return 1;
	for (const auto& monitor : monitors)
	{
		std::cout << monitor.szDevice << ' ' << monitor.size << (monitor.isPrimary() ? " Primary\n" : "\n");
		std::cout << "VirtualCoords: " << monitor.rcMonitor << "\nImageCoords: " << monitor.pixelSpaceCoords << '\n';
	}
	if (fs::exists(path) && fs::is_directory(path))
	{
		std::cout << "Scanning " << path << " ...\n";

		unsigned int cropped = 0;
		for (auto const& entry : fs::directory_iterator(path))
		{
			if (fs::is_regular_file(entry) && entry.path().extension() == ".png")
			{
				if (imageSizeEqualsExtDisplay(entry.path()))
				{
					try 
					{
						cv::Mat image = cv::imread(entry.path().string());
						if (image.empty())
						{
							std::cout << "Cannot find/open image\n";
							continue;
						}
						std::cout << "Cropping " << entry.path() << " ...";
						cv::Mat croppedImage(image, monitors.at(cropDisplay - 1).getCVRect());
						cv::imwrite(entry.path().string(), croppedImage);
						std::cout << " Done\n";
						++cropped;
					}
					catch (cv::Exception ex) { std::cout << '\n' << ex.what() << '\n'; }
				}
			}
		}
		std::cout << "Done, cropped " << cropped << " images!\n";
	}
	else std::cout << "Cannot find specified path/directory\n";
#ifndef _DEBUG
	}
	else std::cout << "Invalid number of arguments\n";
#else
	std::cin.get();
#endif
}