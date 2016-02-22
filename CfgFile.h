/*
 * mpUtils
 * Config.h
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the Config class, which can read, edit and create "init style" configuration files
 *
 * Copyright 2016 Hendrik Schwanekamp
 *
 */

#ifndef MPUTILS_CONFIG_H
#define MPUTILS_CONFIG_H

// includes
//--------------------
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include "Log.h"
//--------------------

// namespace
//--------------------
namespace mpu
{
//--------------------

//-------------------------------------------------------------------
/**
 * class CfgFile
 *  This class manages "init style" configuration files
 *
 * usage:
 *
 */
class CfgFile
{
public:
    CfgFile(const std::string &sName = ""); // constructor
    ~CfgFile(); // destructor

    void open(const std::string &sName); // open config file
    void close();

    template<typename T>
    T getValue(const std::string &sBlock, const std::string &sKey); // get a value from the config file

    // typedefs for the listing functions
    typedef std::vector<std::string> blockList;
    typedef std::unordered_map<std::string, std::string> blockMap;
    typedef std::pair<std::string, blockMap> blockPair;
    typedef std::vector<blockPair> configList;

    blockList getBlockList();   // returns a blockList (an std::vector<string>) which contains all Block names in the config file
    blockMap getBlockMap(const std::string &sBlock); // returns a block map (std::unordered_map<std::string, std::string>) which contains all Key=Value pairs of the specified Block (Values as strings, use mpu::value from string)
    configList getConfigList(); // returns a vector of pairs of block names (as string) and blockMaps for every block in the config file

//    template<typename T>
//    void setValue(std::string sBlock, std::string sKey, T value);
//    addComment();

 private:
    std::string sFilename; // the filename
    std::ifstream cfgfile; // stream to access the config file
    std::unordered_map<std::string, std::streampos> blockPositionCache; // cache the block positions locally (the position after the block)
    std::string sCurrentBlock; // save the name of the block we are currently in to minimize block searching

    int findBlock(const std::string &sBlock); // set the file position pointer right after the block header returns 0 when the block was found
    int getKeyValue(std::string &sLine,
                    const size_t startPos = 0); // look for a key value in sLine if found the value is stored in sLine and 0 is returned. starting at start pos
};

// define all the inline and template functions of the class
//--------------------
template<typename T>
T CfgFile::getValue(const std::string &sBlock, const std::string &sKey)
{
    if(sCurrentBlock != sBlock)
    if(findBlock(sBlock) != 0)
        throw std::invalid_argument("The Block \""+sBlock+"\" does not exist in the Config File " + sFilename); // block is not there

    // some local variables
    std::string sLine;
    size_t nonEmpty;
    size_t endOfName;

    // now search for the key
    std::streampos start = cfgfile.tellg();
    do
    {
        getline(cfgfile,sLine);

        // erase comments at the end
        cutAfterFirst(sLine, "#");
        // the first non whitespace char
        nonEmpty = sLine.find_first_not_of(" \t");

        if(nonEmpty == std::string::npos)
            continue; // ignore empty lines
        if(sLine[nonEmpty] == '[')
        {
            if(findBlock(sBlock) != 0)
                throw std::invalid_argument("The Key \"" + sKey + "\" does not exist in the Config File " +
                                            sFilename); // block is not there
            continue;
        }

        // we found a key
        // check if it is the one we are looking for
        endOfName = sLine.find_first_of(" =\t", nonEmpty+1);
        if(endOfName == std::string::npos)
        {
            logWARNING << "Syntax error in configuration file! File: " << sFilename <<" Block: " << sBlock << " Key: " << sLine;
            continue; // syntax error, we ignore the block (comment or end of line)
        }

        if(sLine.substr(nonEmpty, endOfName-(nonEmpty)) == sKey)
        {
            // get the value
            if (getKeyValue(sLine, endOfName) != 0)
            {
                logWARNING << "Syntax error in configuration file! File: " << sFilename << " Block: " << sBlock <<
                           " Key: " << sLine;
                continue; // syntax error, we ignore the key maby we find a second match that works
            }

            // return the value
            return valueFromString<T>(sLine);
        }

        // check if the stream is still good
        if(!cfgfile.good())
        {
            if(cfgfile.eof()) // for eof go back to the start of the block
            {
                cfgfile.clear();
                if(findBlock(sBlock) != 0)
                    throw std::invalid_argument("The Block \""+sBlock+"\" does not exist in the Config File " + sFilename); // block is not there
            }
            else
                throw std::runtime_error("Error reading from config file! Filename: " + sFilename);
        }
    }
    while( cfgfile.tellg() != start);

    // we searched the whole block, key is not there
    throw std::invalid_argument("The Key \""+sKey+"\" does not exist in the Config File " + sFilename); // key is not there
}

//--------------------

}
#endif //MPUTILS_CONFIG_H
