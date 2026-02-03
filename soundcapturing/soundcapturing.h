#pragma once

#include <string>
class SoundCapturing {
public:
    SoundCapturing() = default;
    void process(std::string device_name);
};
