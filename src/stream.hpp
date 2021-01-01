#ifndef __STREAM_HPP
#define __STREAM_HPP

#include "utils.hpp"

#include <istream>
#include <iostream>

using namespace std;

/*
 * Stream utility wrapping iostream :
 *  - bit read capabilities
 *  - section len management
 */

namespace grib2dec {

class Stream {
public:
    Stream(istream& fin) : fin(fin) {
        sectionId = sectionLen = sectionRemain = -1;
    }

    bool read(int len) {
        if (len > int(sizeof(data)))
            return false;

        if (len > sectionRemain && sectionLen >= 0)
            return false;

        fin.read(data, len);
        if (!fin) {
            cerr << "Cannot read " << len << " bytes" << endl;
            return false;
        }

        sectionRemain -= len;
        return true;
    }

    bool sectionBegin(int maxLen) {
        sectionId = -1;
        sectionLen = -1;

        if (maxLen < 4)
            return false;

        if (!read(4))
            return false;

        if (data[0] == '7' && data[1] == '7' && data[2] == '7' && data[3] == '7') {
            sectionId = 7; // end section
            sectionLen = 4;
            sectionRemain = 0;
            return true;
        }

        sectionLen = min<int>(len32(), maxLen);
        sectionRemain = sectionLen - 4;

        if (sectionRemain <= 0 || !read(1))
            return false;

        sectionId = data[0];

        return true;
    }

    bool sectionEnd() {
        fin.seekg(sectionRemain, ios_base::cur);
        return !fin.eof();
    }

    uint16_t len16() const {
        return grib2dec::len16(data);
    }

    uint32_t len32() const {
        return grib2dec::len32(data);
    }

    uint64_t len64() const {
        return grib2dec::len64(data);
    }

    char data[64];
    int sectionId;
    istream& fin;
    int sectionLen;
    int sectionRemain;
};

} // grib2dec
#endif
