#pragma once

#include <algorithm> 
#include <csignal>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::vector<double> calculate_z_scores(const std::vector<uint16_t>& values);