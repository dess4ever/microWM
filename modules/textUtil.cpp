#include "textUtil.h"

std::string sanitize_utf8(const std::string input) {
    std::string output;
    output.reserve(input.size());

    for (auto it = input.begin(); it != input.end(); ++it) {
        unsigned char c = static_cast<unsigned char>(*it);

        // Simple ASCII is always valid UTF-8
        if (c <= 0x7F) {
            output.push_back(c);
        }
        // Valid UTF-8 byte sequence lengths are from 2 to 4 bytes
        else if ((c >= 0xC2 && c <= 0xDF) && (it + 1 != input.end()) 
                   && (static_cast<unsigned char>(*(it + 1)) >= 0x80 
                   && static_cast<unsigned char>(*(it + 1)) <= 0xBF)) {
            // Two-byte sequence
            output.push_back(c);
            output.push_back(*(++it));
        }
        else if ((c >= 0xE0 && c <= 0xEF) && (it + 2 < input.end()) 
                  && (static_cast<unsigned char>(*(it + 1)) >= 0x80 
                  && static_cast<unsigned char>(*(it + 1)) <= 0xBF) 
                  && (static_cast<unsigned char>(*(it + 2)) >= 0x80 
                  && static_cast<unsigned char>(*(it + 2)) <= 0xBF)) {
            // Three-byte sequence
            output.push_back(c);
            output.push_back(*(++it));
            output.push_back(*(++it));
        }
        else if ((c >= 0xF0 && c <= 0xF4) && (it + 3 < input.end()) 
                  && (static_cast<unsigned char>(*(it + 1)) >= 0x80 
                  && static_cast<unsigned char>(*(it + 1)) <= 0xBF) 
                  && (static_cast<unsigned char>(*(it + 2)) >= 0x80 
                  && static_cast<unsigned char>(*(it + 2)) <= 0xBF) 
                  && (static_cast<unsigned char>(*(it + 3)) >= 0x80 
                  && static_cast<unsigned char>(*(it + 3)) <= 0xBF)) {
            // Four-byte sequence
            output.push_back(c);
            output.push_back(*(++it));
            output.push_back(*(++it));
            output.push_back(*(++it));
        }
    }

    return output;
}

SocketMessage parseInputToSocketMessage(const std::string &input) {
    SocketMessage message;

    std::istringstream iss(input);
    getline(iss, message.function, ' ');

    if (!iss.eof()) {
        getline(iss, message.jsonArguments);
    }

    return message;
}