#pragma once

#include <string>

std::string MakeTimestampedScreenshotName(const std::string& prefix);
bool SaveNativeScreenshot(const std::string& filename);