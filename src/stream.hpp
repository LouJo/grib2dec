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

    void read(int len) {
        if (len > int(sizeof(data)))
            throw parsing_error("len to read > 16 bytes");

        if (len > sectionRemain && sectionLen >= 0)
            throw parsing_error("len to read > section len");

        fin.read(data, len);
        if (!fin)
            throw parsing_error("end of file");

        sectionRemain -= len;
    }

    int bits(int nbBits) {
        int bitsToRead = nbBits - nbBitsRead;
        if (bitsToRead > 0) {
            int bytesToRead = (bitsToRead + 7) / 8;
            read(bytesToRead);
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

    void bitsEnd() {
        // bytes read is already ok
        nbBitsRead = 0;
        bitsReadValue = 0;
    }

    int bytes(int nbBytes) {
        switch (nbBytes) {
        case 1:
            read(1);
            return data[0];
        case 2:
            return int16_t(len16());
        case 4:
            return int32_t(len32());
        case 8:
            return int64_t(len64());
        default:
            throw parsing_error(string("cannot retrieve ") + to_string(nbBytes) + " of value");
        }
    }

    int byte() {
        read(1);
        return data[0];
    }

    uint16_t len16() {
        read(2);
        return grib2dec::len16(data);
    }

    uint32_t len32() {
        read(4);
        return grib2dec::len32(data);
    }

    uint64_t len64() {
        read(8);
        return grib2dec::len64(data);
    }

    void sectionBegin(int maxLen) {
        sectionId = -1;
        sectionLen = -1;

        if (maxLen < 4)
            throw parsing_error("minimum section len is 4 bytes");

        read(4);

        if (data[0] == '7' && data[1] == '7' && data[2] == '7' && data[3] == '7') {
            sectionId = 8; // end section
            sectionLen = 4;
            sectionRemain = 0;
            return;
        }

        sectionLen = min<int>(grib2dec::len32(data), maxLen);
        sectionRemain = sectionLen - 4;

        if (sectionRemain <= 0)
           throw parsing_error("not enough bytes to read in section");

        sectionId = byte();
    }

    void sectionEnd() {
        fin.seekg(sectionRemain, ios_base::cur);
        if (fin.eof())
            throw parsing_error("file too small for section size");
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
