#include "Monitor.h"
#include "docopt/docopt.h"
#include "spdlog/spdlog.h"
#include <clocale>

static constexpr auto USAGE =  // MSVC's std::regex overflows the stack because of backtracking if USAGE is too big...
R"(ImageCrop.
    Usage:
      ImageCrop -p <path>
      ImageCrop -p <path> [-m <Num>]
      ImageCrop -p <path> [-m <Num>] [-v]
      ImageCrop -p <path> [-m <Num>] [-v] [-r]
      ImageCrop -h

    Options:
      -h         Show this screen.
      -p <path>  Required directory path.
      -m <Num>   Monitor to crop [default: 1].
      -v         Show verbose output [default: false].
      -r         Crop images in subdirectories also [default: false].)";

int main(int argc, char* argv[])
{
	std::map<std::string, docopt::value> args = docopt::docopt(USAGE, { argv + 1, argv + argc }, true);  // ::docopt terminates on exception, ::docopt_parse propagates ex

	const auto path        = args["-p"].asString();
	const auto cropDisplay = args["-m"].asLong();
	const auto displayNum  = GetSystemMetrics(SM_CMONITORS);

	if      (!fs::exists(path) || !fs::is_directory(path)) { spdlog::error("Invalid Directory");  return -1; }
	else if (cropDisplay < 1 || cropDisplay > displayNum)  { spdlog::error("Invalid Monitor ID"); return -1; }

	const auto verbose   = args["-v"].asBool();
	const auto recursive = args["-r"].asBool();
	if (!verbose) spdlog::set_level(spdlog::level::warn);
	spdlog::set_pattern("[%^%l%$] %v");   /*%^ start_color_range, %l level, %$ end_color_range, %v value */

	if (!GetAllMonitorInfo()) { spdlog::error("Unable to obtain monitor setup information");  return -1; }
	for (const auto& monitor : monitors) spdlog::info("{}", monitor);  //verbose only

	// foreign character (multi byte) support - spdlog/console will still display them wrong
	// but opencv will read/write (via fopen) to the file correctly
	std::setlocale(LC_ALL, "en_US.UTF-8");
	spdlog::info("Scanning {}", path);

	unsigned int cropped = 0;
	auto crop = [&cropped, &cropDisplay](const auto& entry) {
		const auto& stringPath = entry.path().string();

		if (entry.is_directory()) spdlog::info("Scanning {}", stringPath);

		try {
			if (!(entry.is_regular_file() && cv::haveImageReader(stringPath) 
				&& IC::imageSizeEqualsExtDisplay(entry.path(), Monitor::systemDisplaySize))) return;

			cv::Mat image = cv::imread(stringPath);
			if (image.empty()) { spdlog::warn("Cannot find/read image {}, skipping", stringPath); return; }

			spdlog::info("Cropping {}", stringPath);
			cv::Mat croppedImage(image, monitors.at(static_cast<std::size_t>(cropDisplay - 1)).getCVRect());
			if (!cv::imwrite(stringPath, croppedImage)) spdlog::warn("Unable to crop {}, skipping", stringPath);
			else ++cropped;
		}
		catch (const std::exception& ex) { spdlog::critical("{} while processing {}", ex.what(), stringPath); }
	};
	if (recursive) { for (const auto& entry : fs::recursive_directory_iterator(path)) crop(entry); }
	else           { for (const auto& entry : fs::directory_iterator(path))           crop(entry); }

	if (!verbose)    spdlog::set_level(spdlog::level::info);
	if (cropped > 0) spdlog::info("Scan Complete, cropped {} images to fit {}!", cropped, monitors.at(static_cast<std::size_t>(cropDisplay - 1)).szDevice);
	else             spdlog::info("Scan Complete, no images fit the crop criteria in {}", path);
}