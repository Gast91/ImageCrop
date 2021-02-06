#include <filesystem>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <winsock.h>
#pragma comment(lib, "Ws2_32.lib")
#include <Windows.h>
#include "opencv2/opencv.hpp"

namespace fs = std::filesystem;

long mapToRange(long value, long currentStart, long currentEnd, long newStart, long newEnd)
{
	if (currentStart == newStart && currentEnd == newEnd) return value;
	double slope = 1.0 * (newEnd - newStart) / (currentEnd - currentStart);
	return static_cast<long>(newStart + floor((slope * (value - currentStart)) + 0.5));
}

struct Monitor : public MONITORINFOEXA
{
	static long minW, maxW, minH, maxH;
	static const cv::Size systemDisplaySize;

	RECT pixelSpaceCoords;
	cv::Size size;

	bool isPrimary() const { return dwFlags == MONITORINFOF_PRIMARY; }

	void calculateMetrics()
	{
		size = { rcMonitor.right - rcMonitor.left, rcMonitor.bottom - rcMonitor.top };
		pixelSpaceCoords = {
			mapToRange(rcMonitor.left  , Monitor::minW, Monitor::maxW, 0, Monitor::systemDisplaySize.width),
			mapToRange(rcMonitor.top   , Monitor::minH, Monitor::maxH, 0, Monitor::systemDisplaySize.height),
			mapToRange(rcMonitor.right , Monitor::minW, Monitor::maxW, 0, Monitor::systemDisplaySize.width),
			mapToRange(rcMonitor.bottom, Monitor::minH, Monitor::maxH, 0, Monitor::systemDisplaySize.height) };
	}

	cv::Rect getCVRect() { return { pixelSpaceCoords.left, pixelSpaceCoords.top, size.width, size.height }; }

	friend std::ostream& operator<<(std::ostream& os, const cv::Size& s)
	{
		os << '[' << s.width << ',' << s.height << ']';
		return os;
	}

	friend std::ostream& operator<<(std::ostream& os, const RECT& coords)
	{
		os << '(' << coords.left << ',' << coords.top << ") (" << coords.right << ',' << coords.bottom << ')';
		return os;
	}

}; long Monitor::minW = 0, Monitor::maxW = 0, Monitor::minH = 0, Monitor::maxH = 0; const cv::Size Monitor::systemDisplaySize{ GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN) };

std::ostream& operator<<(std::ostream& os, const RECT coords)
{
	os << '(' << coords.left << ',' << coords.top << ") (" << coords.right << ',' << coords.bottom << ')';
	return os;
}

std::vector<Monitor> monitors;
BOOL GetAllMonitorInfo()
{
	auto getMonitorInfo = [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{
		MONITORINFOEXA info{};
		info.cbSize = sizeof(MONITORINFOEXA);
		BOOL result = GetMonitorInfoA(hMonitor, &info);
		if (result) monitors.push_back(Monitor{ info });
		Monitor::minW = std::min(Monitor::minW, info.rcMonitor.left);
		Monitor::maxW = std::max(Monitor::maxW, info.rcMonitor.right);
		Monitor::minH = std::min(Monitor::minH, info.rcMonitor.top);
		Monitor::maxH = std::max(Monitor::maxH, info.rcMonitor.bottom);
		return result;
	};
	auto result = EnumDisplayMonitors(NULL, NULL, getMonitorInfo, 0);
	if (result) { for (auto& monitor : monitors) monitor.calculateMetrics(); }
	return result;
}

// Width is a 4Byte int starting at offset 16 in the file header. Height is the next one at position 20
// Both in network order so an ntohl conversion to host order is needed (endian)
bool imageSizeEqualsExtDisplay(const fs::path& path)
{
	std::ifstream in(path);
	cv::Size size;

	in.seekg(16);
	in.read((char*)(&size.width), 4);
	in.read((char*)(&size.height), 4);

	size.width = ntohl(size.width);
	size.height = ntohl(size.height);

	return size == Monitor::systemDisplaySize;
}

int main(int argc, char* argv[])
{
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
		std::cout << "Done, cropped " << cropped << "images!\n";
	}
	else std::cout << "Cannot find specified path/directory\n";
#ifndef _DEBUG
	}
	else std::cout << "Invalid number of arguments\n";
#else
	std::cin.get();
#endif
}