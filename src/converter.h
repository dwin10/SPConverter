/*
  ==============================================================================

    converter.h
    Created: 13/12/2023
    Author:  David Winton

  ==============================================================================
*/

#include <iostream>
#include <sndfile.h>
#include <vector>

#ifndef CONVERTER_H
#define CONVERTER_H

class Converter
{
public:
    void convert(const char* inPath, const char* outPath);

private:
    int subformat;
    sf_count_t rFrames, wFrames;
};

#endif /* CONVERTER_H */
