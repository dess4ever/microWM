#pragma once
#include <algorithm>
#include <string>
#include <iostream>
#include "SocketUnix.h"
std::string sanitize_utf8(const std::string input);
SocketMessage parseInputToSocketMessage(const std::string &input);