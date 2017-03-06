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
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <fstream>
#include "Log.h"
//--------------------

// namespace
//--------------------
namespace mpu
{
//--------------------

//-------------------------------------------------------------------
/**
 * @class CfgFile
 * This class manages "init style" configuration files
 *
 * @usage:
 * To open a existing config file you can pass the filename to the constructor or call the open function.
 * If the file can't be opened (eg. it does not exist) a exception is thrown. You can then try to create a empty file
 * using createAndOpen().
 *
 * the config file Format:
 * The config file is grouped into different "Blocks" whose names must be unique in the entire file. Every block consists of
 * variable number of "Keys" with associated values (a key name must be unique in its Block). The file is saved in text
 * file format and can be read or edited manually.
 * If editing manually keep in mind that '#' indicates a one line comment. To use the '#' character it needs to be escaped
 * ('\#'). Therefore backslashes also needs to be escaped ('\\'). Whitespace in the file is ignored. If you want to save
 * data that contains whitespaces you can surround the string by double quotes (' "hi space" '). Therefore double quotes
 * also needs to be escaped. The functions of this class which provides write accesss to the  file will manage the
 * escaping for you and detect spaces automatically. ;)
 *
 * function overview:
 * Once a File is Opened individual Keys can be read and set using getValue() and setValue(). Every data type with a stream
 * in and stream out operator (<</>>) defined can be stored and loaded to or from the config file. To get a whole Block from
 * the file use getBlockMap(), while getBlockList() will return an std::vector containing all Block names found in the current
 * File. getConfigList() is a combination of both and returns the content of the whole file as a vector of BlockMaps.
 * You can also add a whole Block at once using addBlock() or call removeKey() and removeBlock() to remove Keys or Blocks.
 *
 * speed:
 * Reading the file in order is quite fast. Due to internal caching reading in random order is also ok.
 * When using setValue() or addBlock() adding to the end is ok while adding or overwriting something in the middle of the
 * file is slow. (This is because the only way to delete something from a file is to create a new file and overwrite the old one)
 * The same goes for removeBlock() and removeKey(), both need to copy the whole file.
 *
 * exceptions:
 * If you try to operate on a Key or Block that does not exist a invalid_argument() exception is thrown.
 * invalid_argument() is also thrown if you try to write a string to the file containing a new line character (\n \r).
 * If your config file is totally broken you might get a logic_exception() when trying to edit it.
 * A runtime_error() exception is thrown when a file cannot be opened or created.
 *
 * thread safety:
 * This is not at all thread save. Most functions modify the internal fstream (and the file on the disk) and access from
 * multiple threads will most likely create race conditions.
 *
 */
class CfgFile
{
public:
    CfgFile(const std::string &sName = ""); // constructor

    void open(const std::string &sName); // open config file
    void createAndOpen(const std::string &sName); // creates and then opens s new empty config file, if the file already exists its content is overwritten
    void close();

    // typedefs for the listing functions
    typedef std::vector<std::string> blockList;
    typedef std::unordered_map<std::string, std::string> blockMap;
    typedef std::pair<std::string, blockMap> blockPair;
    typedef std::vector<blockPair> configList;

    template<typename T>
    T getValue(const std::string &sBlock, const std::string &sKey); // get a value from the config file

    blockList getBlockList();   // returns a blockList (an std::vector<string>) which contains all Block names in the config file
    blockMap getBlockMap(const std::string &sBlock); // returns a block map (std::unordered_map<std::string, std::string>) which contains all Key=Value pairs of the specified Block (Values as strings, use mpu::value from string)
    configList getConfigList(); // returns a vector of pairs of block names (as string) and blockMaps for every block in the config file

    template<typename T>
    void setValue(const std::string &sBlock, const std::string &sKey, T value,
                  std::string sComment = ""); // creates the Block if necessary and then creates the key or changes its value and adds the comment

    void addBlock(const std::string &sBlock,
                  const blockMap &block); // adds a Block to the file if it already exists it is overwritten
    void addComment(const std::string &s); // adds a comment at the end of the file

    void removeBlock(
            const std::string &sBlock); // removes a whole block from the file including the comments but leaves the header in the file
    void removeKey(const std::string &sBlock,
                   const std::string &sKey); // removes a Key from the file including the comments

    // make the class non copyable
    CfgFile(const Log& that) = delete;
    CfgFile& operator=(const Log& that) = delete;

 private:
    std::string sFilename; // the filename
    std::fstream cfgfile; // stream to access the config file
    std::unordered_map<std::string, std::streampos> blockPositionCache; // cache the block positions locally (the position after the block)
    std::streampos biggestBlockPosition; // save the biggest block position here
    std::string sCurrentBlock; // save the name of the block we are currently in to minimize block searching

    int findBlock(const std::string &sBlock); // set the file position pointer right after the block header returns 0 when the block was found
    int getKeyValue(std::string &sLine,
                    const size_t startPos = 0); // look for a key value in sLine if found the value is stored in sLine and 0 is returned. starting at start pos
    int findKey(
            const std::string &sKey);   // searches for a Key only to the end of the current block (used by the write functions).
    // If 0 is returned, the streampos is now BEFORE the Line with the key. If 1 is returned,
    // it is on the end of the block and if 2 is returned it is on the end of the file

    void copyFirstPart(std::ofstream &filestream,
                       const std::streampos &to); // opens a new temp file and copys the contend of the cfg file into it up to "to"
    void copySecondPart(std::ofstream &filestream,
                        const std::streampos &from); // copys the cfg file from "from" to eof then closes the temp file and overwrites the cfg file with the tmp file
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
        cutAfterFirst(sLine, "#", "\\");
        // the first non whitespace char
        nonEmpty = sLine.find_first_not_of(" \t");

        if(nonEmpty == std::string::npos)
        {
            // ignore empty lines, but check for eof first
            if (!cfgfile.good())
            {
                if (cfgfile.eof()) // for eof go back to the start of the block
                {
                    cfgfile.clear();
                    if (findBlock(sBlock) != 0)
                        throw std::invalid_argument("The Block \"" + sBlock + "\" does not exist in the Config File " +
                                                    sFilename); // block is not there
                }
                else
                    throw std::runtime_error("Error reading from config file! Filename: " + sFilename);
            }
            continue;
        }


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
            logWARNING("CfgFile") << "Syntax error in configuration file! File: " << sFilename <<" Block: " << sBlock << " Key: " << sLine;
            continue; // syntax error, we ignore the block (comment or end of line)
        }

        if(sLine.substr(nonEmpty, endOfName-(nonEmpty)) == sKey)
        {
            // get the value
            if (getKeyValue(sLine, endOfName) != 0)
            {
                logWARNING("CfgFile") << "Syntax error in configuration file! File: " << sFilename << " Block: " << sBlock <<
                           " Key: " << sLine;
                continue; // syntax error, we ignore the key maby we find a second match that works
            }

            // return the value
            return fromString<T>(sLine);
        }
    }
    while( cfgfile.tellg() != start);

    // we searched the whole block, key is not there
    throw std::invalid_argument("The Key \""+sKey+"\" does not exist in the Config File " + sFilename); // key is not there
}

template<typename T>
void CfgFile::setValue(const std::string &sBlock, const std::string &sKey, T value, std::string sComment)
{
    std::string sValue = toString(value);

    if (!sComment.empty())
        sComment = "\t# " + sComment;

    if (sValue.find_first_of("\n\r") != std::string::npos)
        throw std::invalid_argument("I can't write a string containing a 'new line' to the config file!");

    escapeString(sValue, "\"#", '\\');
    if (sValue.find_first_of(" \t") != std::string::npos)
        sValue = "\"" + sValue + "\"";

    if (findBlock(sBlock) == 0)
    {
        // block was found
        switch (findKey(sKey))
        {
        case 0:
        {
            // found the key
            std::ofstream f;
            copyFirstPart(f, cfgfile.tellg());
            f << '\t' << sKey << " = " << sValue << sComment << "\n";
            cfgfile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            copySecondPart(f, cfgfile.tellg());
            break;
        }
        case 1:
        {
            // end of block
            std::ofstream f;
            copyFirstPart(f, cfgfile.tellg());
            f << '\t' << sKey << " = " << sValue << sComment << "\n";
            copySecondPart(f, cfgfile.tellg());
            break;
        }
        case 2:
            // eof, so just create the key
            cfgfile.unget();
            if (cfgfile.get() != '\n')
                cfgfile << '\n';
            cfgfile.clear();

            cfgfile << '\t' << sKey << " = " << sValue << sComment << "\n";
            cfgfile.flush();
            break;

        default:
            throw std::logic_error("unhandled return value of findKey()");
        }

    }
    else
    {
        // block is not in the file create the block at the end of the file
        // go to the last block in the file and then search a non existing value to to the position after to the block
        cfgfile.seekg(biggestBlockPosition);

        if (findKey("") != 2)
            throw std::logic_error(
                    "unhandled return value of findKey(). Or something is very wrong with your cfg file.");

        // now we are at eof, but empty lines or comments at the end of the file do not matter
        cfgfile.unget();
        if (cfgfile.get() != '\n')
            cfgfile << '\n';
        cfgfile.clear();

        cfgfile << "\n[" << sBlock << "]\n";
        biggestBlockPosition = cfgfile.tellp();
        blockPositionCache[sBlock] = biggestBlockPosition;
        cfgfile << "\t" << sKey << " = " << sValue << sComment << "\n";
        cfgfile.flush();
    }
}

//--------------------

}
#endif //MPUTILS_CONFIG_H
