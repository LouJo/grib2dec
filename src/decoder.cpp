#include "decoder.hpp"
#include "sections.hpp"
#include "struct.hpp"

#include <fstream>
#include <iostream>
#include <string.h>

using namespace std;

namespace grib2dec {
namespace {

void readMessage(istream& fin, Message& message, vector<double>& values)
{
    Stream stream(fin);
    values.clear();

    readIndicatorSection(stream, message);

    while (!message.complete && message.lenRead < message.len)
        readSection(stream, message, values);
}

void convertMessage(const Message& message, G2DEC_Message& output)
{
    output.datetime = message.datetime;
    output.discipline = message.discipline;
    output.category = message.category;
    output.parameter = message.parameter;
}

} // local namespace

Decoder::Decoder(istream& fin)
    : fin(fin)
{
}

Decoder::Decoder(const char *filename)
    : fin(fileStream)
{
    fileStream.open(filename, ios_base::in | ios_base::binary);
    if (!fileStream.is_open())
        throw file_open_error();
}

G2DEC_Status Decoder::nextMessage(G2DEC_Message& output)
{
    memset(&output, 0, sizeof(output));

    if (ended)
        return G2DEC_STATUS_END;

    fin.seekg(nextMessagePos, ios_base::beg);

    // test end of file
    {
        char c = fin.get();
        if (fin.eof()) {
            ended = true;
            return G2DEC_STATUS_END;
        }
        fin.putback(c);
    }

    Message message;

    try {
        readMessage(fin, message, values);
    } catch (const parsing_error& e) {
        cerr << e.what() << endl;

        if (message.len == 0)
            ended = true;
        else
            nextMessagePos += message.len;

        return e.status();
    }

    convertMessage(message, output);
    output.values = values.data();
    output.valuesLength = values.size();

    if (message.len == 0) // shouldn't occur
        ended = true;
    else
        nextMessagePos += message.len;

    return G2DEC_STATUS_OK;
}

Grib2Dec *Grib2Dec::create(istream& fin)
{
    return new Decoder(fin);
}

Grib2Dec *Grib2Dec::create(const char *filename)
{
    try {
        return new Decoder(filename);
    } catch (const file_open_error& e) {
        cerr << "cannot open file " << filename << endl;
        return nullptr;
    }
}

} // grib2dec
