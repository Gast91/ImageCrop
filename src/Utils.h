#pragma once
#include <filesystem>
#include <fstream>
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN       
#include <winsock.h>
#pragma comment(lib, "Ws2_32.lib")      
#include <Windows.h>
#include "opencv2/opencv.hpp"

namespace fs = std::filesystem;

namespace IC
{
	long mapToRange(long value, long currentStart, long currentEnd, long newStart, long newEnd)
	{
		if (currentStart == newStart && currentEnd == newEnd) return value;
		double slope = 1.0 * (newEnd - newStart) / (currentEnd - currentStart);
		return static_cast<long>(newStart + floor((slope * (value - currentStart)) + 0.5));
	}

	// ONLY FOR PNGs
    // Width is a 4Byte int starting at offset 16 in the file header. Height is the next one at position 20
    // Both in network order so an ntohl conversion to host order is needed (endian)
	bool imageSizeEqualsExtDisplay(const fs::path& path, const cv::Size& compareSize)
	{
		std::ifstream in(path);
		cv::Size size;

		in.seekg(16);
		in.read((char*)(&size.width), 4);
		in.read((char*)(&size.height), 4);

		size.width = ntohl(size.width);
		size.height = ntohl(size.height);

		return size == compareSize;
	}

	// MAYBE TODO:
    //		ADD JPEG
	//		ADD GIF
	//		ADD BMP
}