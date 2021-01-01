#ifndef __STREAM_HPP
#define __STREAM_HPP

#include "utils.hpp"

#include <istream>
#include <iostream>
#include <assert.h>

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

    int bits(int nbBits) {
        int bitsToRead = nbBits - nbBitsRead;
        if (bitsToRead > 0) {
            int bytesToRead = (bitsToRead + 7) / 8;
            if (!read(bytesToRead))
                return -1;
            bitsReadValue <<= bytesToRead * 8;
            nbBitsRead += bytesToRead * 8;
            for (int i = 0; i < bytesToRead; i++)
                bitsReadValue |= uint64_t(((uint8_t*)data)[i]) << ((bytesToRead - i - 1) * 8);
        }

        assert(nbBits <= nbBitsRead);

        int v = bitsReadValue >> (nbBitsRead - nbBits);
        nbBitsRead -= nbBits;
        bitsReadValue = bitsReadValue & ((1 << nbBits) - 1);
        return v;
    }

    int bytes(int nbBytes) {
        if (!read(nbBytes))
            return -1;
        switch (nbBytes) {
        case 1:
            return data[0];
        case 2:
            return int16_t(len16());
        case 4:
            return int32_t(len32());
        case 8:
            return int64_t(len64());
        }
    }

    void bitsEnd() {
        // bytes read is already ok
        nbBitsRead = 0;
        bitsReadValue = 0;
    }

    bool sectionBegin(int maxLen) {
        sectionId = -1;
        sectionLen = -1;

        if (maxLen < 4)
            return false;

        if (!read(4))
            return false;

        if (data[0] == '7' && data[1] == '7' && data[2] == '7' && data[3] == '7') {
            sectionId = 8; // end section
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
    int sectionId = -1;
    istream& fin;
    int sectionLen = -1;
    int sectionRemain = -1;

    // read bits status
    int nbBitsRead = 0;
    uint64_t bitsReadValue = 0;
};

} // grib2dec
#endif
