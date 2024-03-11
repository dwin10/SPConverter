/*
  ==============================================================================

    converter.cpp
    Created: 13/12/2023
    Author:  David Winton

  ==============================================================================
*/

#include "converter.h"
#include <filesystem>
#include <fstream>
#include "includes/r8brain/CDSPResampler.h"

namespace fs = std::filesystem;

/**
 * @brief Copies a file from a source path to destination path.
 * @param sourcePath Source path of the file.
 * @param destinationPath Destination path for the copied file.
 */
void copyFile(const std::string& sourcePath, const std::string& destinationPath) {
    try {
        std::ifstream sourceFile(sourcePath, std::ios::binary);
        if (!sourceFile.is_open()) {
            throw std::runtime_error("Failed to open source file");
        }

        std::ofstream destinationFile(destinationPath, std::ios::binary);
        if (!destinationFile.is_open()) {
            throw std::runtime_error("Failed to open destination file");
        }

        destinationFile << sourceFile.rdbuf();
    } catch (const std::exception& e) {
        std::cerr << "Error copying file: " << e.what() << std::endl;
    }
}

/**
 * @brief Converts a file to a 16 bit wav.
 * Main converter method for the Converter class. 
 * Takes an input path and either processes it or copies it. 
 * @param inPath Path of the file to check/process.
 * @param outPath Path that the processed file will be written to.
 */
void Converter::convert(const char* inPath, const char* outPath)
{
    SF_INFO sfinfo;
    SNDFILE *inFile = sf_open(inPath, SFM_READ, &sfinfo);

    if (!inFile) {
        std::cerr << "Error opening the input file." << std::endl;
        return;
    }

    // Check the subformat mask to check if the file is already 16 bit
    // If true, then use copyFile and close inFile.
    subformat = sfinfo.format & SF_FORMAT_SUBMASK;
    if (subformat == SF_FORMAT_PCM_16) {
        std::cout << "[!] File is already 16 bit. Copying instead.." << std::endl;
        copyFile(inPath, outPath);
        sf_close(inFile);
        return;
    } 

    // Allocate memory in fArray
    std::vector<double> fArray(sfinfo.frames * sfinfo.channels);
    double* fData = fArray.data();

    // Read the frames to fArray
    rFrames = sf_readf_double(inFile, fData, sfinfo.frames);

    // Close the input file as it is no longer needed
    sf_close(inFile);

    // Set the format to WAV_PCM_16 for writing
    sfinfo.samplerate = 48000;
    sfinfo.channels = 2;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    //Open the outfile
    SNDFILE *outFile = sf_open(outPath, SFM_WRITE, &sfinfo);

    // Resample with r8brain 
    r8b::CDSPResampler16 resampler(sfinfo.samplerate, sfinfo.samplerate, fArray.size());
    resampler.process(fData, fArray.size(), fData);

    // Write the converter frames to 
    wFrames = sf_write_double(outFile, &fArray[0], fArray.size());

    // Free the allocated memory and close the outfile
    sf_close(outFile);
    fArray.clear();
}
