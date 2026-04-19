#include "screenshot.h"

#include "raylib.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace
{
std::tm LocalTime(std::time_t timeValue)
{
    std::tm tmValue{};
#if defined(_WIN32)
    localtime_s(&tmValue, &timeValue);
#else
    localtime_r(&timeValue, &tmValue);
#endif
    return tmValue;
}
} // namespace

std::string MakeTimestampedScreenshotName(const std::string& prefix)
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t timeValue = std::chrono::system_clock::to_time_t(now);
    const std::tm tmValue = LocalTime(timeValue);

    std::ostringstream output;
    output << prefix << "-" << std::put_time(&tmValue, "%Y%m%d-%H%M%S") << ".png";
    return output.str();
}

bool SaveNativeScreenshot(const std::string& filename)
{
    if (!IsWindowReady())
    {
        return false;
    }

    TakeScreenshot(filename.c_str());
    return true;
}