#pragma once
#include <filesystem>
#include <fstream>
#include <vector>
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN       
#include <winsock.h>
#pragma comment(lib, "Ws2_32.lib")      
#include <Windows.h>
#include "opencv2/opencv.hpp"
#include "spdlog/fmt/ostr.h"

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

	template<typename Ostream>
	friend Ostream& operator<<(Ostream& os, const cv::Size& s)
	{
		return os << '[' << s.width << ',' << s.height << ']';
	}

	template<typename Ostream>
	friend Ostream& operator<<(Ostream& os, const RECT& coords)
	{
		return os << '(' << coords.left << ',' << coords.top << ") (" << coords.right << ',' << coords.bottom << ')';
	}

	template<typename Ostream>
	friend Ostream& operator<<(Ostream& os, const Monitor& monitor)
	{
		return os << monitor.szDevice << ' ' << monitor.size << (monitor.isPrimary() ? " Primary" : "") << "\n\tVirtualCoords: "
			<< monitor.rcMonitor << "\n\tImageCoords: " << monitor.pixelSpaceCoords;
	}

}; long Monitor::minW = 0, Monitor::maxW = 0, Monitor::minH = 0, Monitor::maxH = 0; const cv::Size Monitor::systemDisplaySize{ GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN) };

template<typename Ostream>
Ostream& operator<<(Ostream& os, const RECT coords)
{
	return os << '(' << coords.left << ',' << coords.top << ") (" << coords.right << ',' << coords.bottom << ')';
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