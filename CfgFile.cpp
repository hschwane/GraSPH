/*
 * mpUtils
 * Config.cpp
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Implements the CfgFile class, which can read, edit and create "init style" configuration files
 *
 * Copyright 2016 Hendrik Schwanekamp
 *
 */

// includes
//--------------------
#include "CfgFile.h"
//--------------------

// namespace
//--------------------
namespace mpu {
//--------------------

// function definitions of the CfgFile class
//-------------------------------------------------------------------

CfgFile::CfgFile(const std::string &sName)
{
    if(!sName.empty())
    {
        open(sName);
    }
}

CfgFile::~CfgFile()
{
    // file will autoclose in its destructor
}

void CfgFile::open(const std::string &sName)
{
    cfgfile.open(sName, std::ifstream::in );
    if(!cfgfile.is_open())
        throw std::runtime_error("Could not open config file: " + sName);
    sFilename = sName;
}

void CfgFile::close()
{
    if(cfgfile.is_open())
        cfgfile.close();

    sCurrentBlock = "";
    blockPositionCache.clear();
    sFilename = "";
}

int CfgFile::findBlock(const std::string &sBlock)
{
    // see if its already in the cache
    if(blockPositionCache.count( sBlock) > 0)
    {
        // found it set the stream position
        cfgfile.seekg(blockPositionCache.at(sBlock));
        sCurrentBlock = sBlock;
        return 0;
    }

    // some local variables
    std::string sLine;
    size_t nonEmpty;
    size_t closingBracket;

    // we need to search manually start where we currently are
    std::streampos start = cfgfile.tellg();
    do
    {
        getline(cfgfile,sLine);

        // the first non whitespace char
        nonEmpty = sLine.find_first_not_of(" \t");

        // now check if the current line is a block
        if(sLine[nonEmpty] == '[')
        {
            // find the closing bracket
            closingBracket = sLine.find_first_of("]# ");
            if( sLine[closingBracket] != ']')
            {
                logWARNING << "Syntax error in configuration file! File: " << sFilename <<" Block: " << sLine;
                continue; // syntax error, we ignore the block (comment or end of line)
            }

            // now check if what we got is what we are looking for
            nonEmpty++; // ignore the [
            if(sLine.substr(nonEmpty, closingBracket-(nonEmpty)) == sBlock)
            {
                blockPositionCache[sBlock] = cfgfile.tellg();
                sCurrentBlock = sBlock;
                return 0; // we found it
            }
            else // cache the position
                blockPositionCache[sLine.substr(nonEmpty, closingBracket-(nonEmpty))] = cfgfile.tellg();
        }

        // check if the stream is still good
        if(!cfgfile.good())
        {
            if(cfgfile.eof()) // for eof go back to the start
            {
                cfgfile.clear();
                cfgfile.seekg( 0,std::ios::beg);
            }
            else
                throw std::runtime_error("Error reading from config file! Filename: " + sFilename);
        }
    }
    while( cfgfile.tellg() != start);

    // didnt find anything
    return -1;
}

CfgFile::blockList CfgFile::getBlockList()
{
    // some local variables
    std::string sLine;
    std::string sBlock;
    size_t nonEmpty;
    size_t closingBracket;
    blockList resultVec;

    // find the biggest block in the cache to determine the starting position
    std::streampos pos = std::ios::beg;
    for (auto block : blockPositionCache)
    {
        resultVec.push_back(block.first);
        if(block.second > pos)
            pos = block.second;
    }

    // set the position
    cfgfile.seekg(pos);

    // now get all the others
    while(cfgfile.good())
    {
        getline(cfgfile,sLine);

        // the first non whitespace char
        nonEmpty = sLine.find_first_not_of(" \t");

        // check if the current line is a block
        if(sLine[nonEmpty] == '[')
        {
            // find the closing bracket
            closingBracket = sLine.find_first_of("]# ");
            if( sLine[closingBracket] != ']')
            {
                logWARNING << "Syntax error in configuration file! File: " << sFilename <<" Block: " << sLine;
                continue; // syntax error, we ignore the block (comment or end of line)
            }

            // we found a valid block, add it to the list
            nonEmpty++; // ignore the [
            sBlock = sLine.substr(nonEmpty, closingBracket - (nonEmpty));
            resultVec.push_back(sBlock);
            blockPositionCache[sBlock] = cfgfile.tellg();
        }
    }

    if(!cfgfile.eof())
        throw std::runtime_error("Error reading from config file! Filename: " + sFilename);

    cfgfile.clear();
    cfgfile.seekg( 0,std::ios::beg);
    sCurrentBlock = ""; // we are at 0, so in no block

    return resultVec;
}

CfgFile::blockMap CfgFile::getBlockMap(const std::string &sBlock)
{
    // first find the block
    if(findBlock(sBlock) != 0)
        throw std::invalid_argument("The Block \""+sBlock+"\" does not exist in the Config File " + sFilename); // block is not there

    // some local variables
    std::string sLine;
    std::string sValue;
    size_t nonEmpty;
    size_t endOfName;
    blockMap resultMap;

    // then get all the keys and values
    while(cfgfile.good())
    {
        getline(cfgfile,sLine);

        // erase comments at the end
        cutAfterFirst(sLine, "#");
        // the first non whitespace char
        nonEmpty = sLine.find_first_not_of(" \t");

        if(nonEmpty == std::string::npos)
            continue; // ignore empty lines
        if(sLine[nonEmpty] == '[')
            break; // we are done this is the next block

        // we found a key
        // see where the name is
        endOfName = sLine.find_first_of(" =\t", nonEmpty+1);
        if(endOfName == std::string::npos)
        {
            logWARNING << "Syntax error in configuration file! File: " << sFilename <<" Block: " << sBlock << " Key: " << sLine;
            continue; // syntax error, we ignore the block (comment or end of line)
        }

        // get the value
        sValue = sLine;
        if (getKeyValue(sValue, endOfName) != 0)
        {
            logWARNING << "Syntax error in configuration file! File: " << sFilename << " Block: " << sBlock <<
                       " Key: " << sLine;
            continue; // syntax error, we ignore the key
        }


        resultMap[sLine.substr(nonEmpty, endOfName-(nonEmpty))] = sValue;
    }

    if( !cfgfile.good())
    {
        if(!cfgfile.eof())
            throw std::runtime_error("Error reading from config file! Filename: " + sFilename);
        cfgfile.clear();
        cfgfile.seekg( 0,std::ios::beg);
    }

    sCurrentBlock = ""; // we have no idea where we are

    return resultMap;
}

CfgFile::configList CfgFile::getConfigList()
{
    // some local variables
    std::string sLine;
    std::string sThisBlocksName;
    std::string sValue;
    size_t nonEmpty;
    size_t closingBracket;
    size_t endOfName;
    configList resultVec;
    blockMap tmpMap;

    // go to the start
    cfgfile.seekg( 0,std::ios::beg);

    // and parse the whole file
    while(cfgfile.good())
    {
        getline(cfgfile,sLine);
        cutAfterFirst(sLine, "#");

        // the first non whitespace char
        nonEmpty = sLine.find_first_not_of(" \t");

        // check what we got
        if(nonEmpty == std::string::npos) // nothing
            continue;
        else if(sLine[nonEmpty] == '[') // a new block
        {
            // find the closing bracket
            closingBracket = sLine.find("]");
            if( closingBracket == std::string::npos)
            {
                logWARNING << "Syntax error in configuration file! File: " << sFilename <<" Block: " << sLine;
                continue; // syntax error, we ignore the block (comment or end of line)
            }

            // we found the next block
            if( !sThisBlocksName.empty())
            {
                resultVec.push_back(blockPair(sThisBlocksName,tmpMap));
                tmpMap.clear();
            }

            nonEmpty++; // ignore the [
            sThisBlocksName = sLine.substr(nonEmpty, closingBracket-(nonEmpty));
            blockPositionCache[sThisBlocksName] = cfgfile.tellg();
        }
        else if( !sThisBlocksName.empty()) // a key (ignore keys before first block)
        {
            // add the key to tmpMap

            // find the key name
            endOfName = sLine.find_first_of(" =\t", nonEmpty+1);
            if(endOfName == std::string::npos)
            {
                logWARNING << "Syntax error in configuration file! File: " << sFilename <<" Block: " << sThisBlocksName << " Key: " << sLine;
                continue; // syntax error, we ignore the key
            }

            // get the value
            sValue = sLine;
            if (getKeyValue(sValue, endOfName) != 0)
            {
                logWARNING << "Syntax error in configuration file! File: " << sFilename << " Block: " <<
                           sThisBlocksName << " Key: " << sLine;
                continue; // syntax error, we ignore the key
            }

            tmpMap[sLine.substr(nonEmpty, endOfName-(nonEmpty))] = sValue;
        }
    }

    if(!cfgfile.eof())
        throw std::runtime_error("Error reading from config file! Filename: " + sFilename);

    // put in the last block
    resultVec.push_back(blockPair(sThisBlocksName,tmpMap));

    cfgfile.clear();
    cfgfile.seekg( 0,std::ios::beg);
    sCurrentBlock = ""; // we are at 0, so in no block

    return resultVec;
}

int CfgFile::getKeyValue(std::string &sLine, const size_t startPos)
{
    // we found the key now get the value
    size_t valueBegin = sLine.find_first_not_of(" =\t", startPos);
    if (valueBegin == std::string::npos)
        return 1; // syntax error, we ignore the block (comment or end of line)

    size_t valueEnd = std::string::npos;
    if (sLine[valueBegin] == '\"')
    {
        // find a second '"' which is not escaped and remove all '\' which are not escaped
        valueBegin++;
        size_t find = sLine.find_first_of("\"\\", valueBegin);
        while (find != std::string::npos)
        {
            if (sLine[find] == '\"')
            {
                valueEnd = find;
                break;
            }
            else
            {
                sLine.erase(find, 1);
                find += 2;
            }
            find = sLine.find_first_of("\"\\", find);
        }

        if (valueEnd == std::string::npos)
            return 1; // syntax error, we ignore the block (comment or end of line)

        sLine = sLine.substr(valueBegin, valueEnd - valueBegin);
    }
    else
    {
        valueEnd = sLine.find_last_not_of(" \t");
        if (valueEnd == std::string::npos)
            return 1; // syntax error, we ignore the block (comment or end of line)

        valueEnd++;
        sLine = sLine.substr(valueBegin, valueEnd - valueBegin);
        removeWhite(sLine);
    }

    return 0;
}

}