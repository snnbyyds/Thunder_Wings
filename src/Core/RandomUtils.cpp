#include "Core/RandomUtils.hpp"

std::random_device RandomUtils::rd;
std::mt19937 RandomUtils::gen(rd());
