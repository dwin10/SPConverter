/*
  ==============================================================================

    main.cpp
    Created: 11/03/2024
    Author:  David Winton

  ==============================================================================
*/

#include <iostream>
#include <chrono>
#include <filesystem>
#include "converter.h"

namespace fs = std::filesystem;

// Set the allowed file extensions that SPconverter can process
std::vector<std::string> allowedExtensions = {".wav", ".flac", ".ogg", ".mp3"};

/**
 * @brief Gets the output path for a given input path.
 * Takes the input path and applies "-SPC" to the file name
 * to differentiate between the original file and the 
 * converted file.
 * 
 * @param inPath std::string containing the input path.
 * @return std::string containing the output path.
 */
std::string getOutPath(std::string inPath)
{
    /*
    Takes the input path and creates an output path with -SPC added to the filename
    */
    fs::path iPath(inPath);
    fs::path dir    = iPath.parent_path().string();
    fs::path fName  = iPath.stem().string() + "-SPC" + iPath.extension().string();
    fs::path oPath  = dir / fName;
    return oPath;
}

/**
 * @brief Confirms a file has an allowed extension.
 * @param filePath Path of the file to check/process.
 * @return bool containing the result.
 */
bool hasAllowedExtension(const std::string& filePath) {
    fs::path path(filePath);
    std::string extension = path.extension().string();

    for (const std::string& allowedExt : allowedExtensions) {
        if (extension == allowedExt) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Gets a list of all files in a directory.
 * @param dirIterator std::filesystem directory iterator type
 * @param filePaths The vector in which retrieved file paths will be stored
 */
template <typename Iterator>
void getFilePaths(const Iterator& dirIterator, std::vector<std::string>& filePaths) {
    for (const auto& entry : dirIterator) {
        if (fs::is_regular_file(entry) && hasAllowedExtension(entry.path().string())) {
            filePaths.push_back(entry.path().string());
        }
    }
}

/**
 * @brief Generates a progress string for output to the terminal.
 * @param inPath Path of the file to check/process.
 * @param currentPos Current index position.
 * @param listSize Size of the file list.
 * @return std::string containing the formatted progress string.
 */
std::string getProgressStr(std::string inPath, int currentPos, int listSize)
{
    return "Converting.. [" + std::to_string(currentPos)+ "/" + std::to_string(listSize) + "].. " + inPath;
}

/**
 * @brief Processes a single filePath.
 * Converts the file with SPconverter.
 * @param filePath Path of the file to check/process.
 * @param conv The SPconverter object used for conversion.
 */
void processFile(const std::string& filePath, Converter& conv) {
    std::string outPath = getOutPath(filePath);
    conv.convert(filePath.c_str(), outPath.c_str());
}

/**
 * @brief Processes a single filePath.
 * Converts the file with SPconverter but allows the explicit setting of outFilePath 
 * @param filePath Path of the file to check/process.
 * @param outFilePath Path that the processed file will be written to.
 * @param conv The SPconverter object used for conversion.
 */
void processFile(const std::string& filePath, const std::string outFilePath, Converter& conv) {
    conv.convert(filePath.c_str(), outFilePath.c_str());
}

/**
 * @brief Processes a directory.
 * This method does a scan of all files (deep/recursive mode can be set
 * with the boolean recurseMode parameter) and converts all valid file paths with SPconverter
 * @param inPath Path of the file to check/process.
 * @param conv The SPconverter object used for conversion.
 * @param recurseMode Sets recursive mode on/off.
 */
void processDirectory(const fs::path& inPath, Converter& conv, bool recurseMode) {
    // Initialize fileList 
    std::vector<std::string> fileList;

    // Set std::filesystem iterator type based on recurse mode
    if (recurseMode) {
        getFilePaths(fs::recursive_directory_iterator(inPath), fileList);
    } else {
        getFilePaths(fs::directory_iterator(inPath), fileList);
    }

    // Set the file counter to initial state
    int fileCounter = 1;

    // Create a new directory with "-SPC" appended to the original directory name
    fs::path convertedDir = inPath.parent_path() / (inPath.filename().string() + "-SPC");
    fs::create_directory(convertedDir);

    // Process all file paths in fileList
    for (const auto& filePath : fileList) {
        std::cout << getProgressStr(filePath, fileCounter, fileList.size()) << std::endl;

        // Construct the output path for the converted file in the new directory
        std::string fileName = fs::path(filePath).filename().string();

        // Construct the output path in the new directory
        std::string relativePath = fs::relative(filePath, inPath).string();
        fs::path outFilePath = convertedDir / relativePath;

        // Ensure the parent directory exists for the output file
        fs::create_directories(outFilePath.parent_path());

        // Process the file using the old file path for input and the new directory for output
        processFile(filePath, outFilePath, conv);
    
        // Iterate fileCounter
        fileCounter++;
    }
}

int main(int argc, char* argv[]) {

    // Record the start time
    auto start = std::chrono::high_resolution_clock::now();

    // Setup required params 
    std::string inPath, outPath;
    inPath = argv[1];
    bool recurseMode = true;

    // Initialise the converter
    Converter spconverter;

    // Validate all neccessary paths and convert
    if (fs::exists(inPath)) {
        if (fs::is_regular_file(inPath) && hasAllowedExtension(inPath)) {
            processFile(inPath, spconverter);
        } else if (fs::is_directory(inPath)) {
            processDirectory(inPath, spconverter, recurseMode);
        } else {
            std::cout << inPath << " is neither a regular file nor a directory." << std::endl;
        }
    } else {
        std::cout << inPath << " does not exist." << std::endl;
    }

    // Record the end time
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the duration
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // Print the duration in microseconds
    std::cout << "Execution Time: " << duration.count() << " microseconds" << std::endl;

    return 0;
}
