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
    biggestBlockPosition = cfgfile.beg;

    if(!sName.empty())
        open(sName);
}

CfgFile::~CfgFile()
{
    // file will autoclose in its destructor
}

void CfgFile::open(const std::string &sName)
{
    cfgfile.open(sName, std::fstream::in | std::fstream::out);
    if(!cfgfile.is_open())
        throw std::runtime_error("Could not open config file: " + sName);
    sFilename = sName;
    cfgfile << std::boolalpha;
}

void CfgFile::createAndOpen(const std::string &sName)
{
    cfgfile.open(sName, std::fstream::in | std::fstream::out | std::fstream::trunc);
    if (!cfgfile.is_open())
        throw std::runtime_error("Could not open or create config file: " + sName);
    sFilename = sName;
    cfgfile << std::boolalpha;
}

void CfgFile::close()
{
    if(cfgfile.is_open())
        cfgfile.close();

    sCurrentBlock = "";
    blockPositionCache.clear();
    biggestBlockPosition = cfgfile.beg;
}

CfgFile::blockList CfgFile::getBlockList()
{
    // some local variables
    std::string sLine;
    std::string sBlock;
    size_t nonEmpty;
    size_t closingBracket;
    blockList resultVec(blockPositionCache.size());

    // put everything cached in the vector
    for (auto block : blockPositionCache)
        resultVec.push_back(block.first);

    // set the position
    cfgfile.seekg(biggestBlockPosition);

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
                logWARNING("CfgFile") << "Syntax error in configuration file! File: " << sFilename <<" Block: " << sLine;
                continue; // syntax error, we ignore the block (comment or end of line)
            }

            // we found a valid block, add it to the list
            nonEmpty++; // ignore the [
            sBlock = sLine.substr(nonEmpty, closingBracket - (nonEmpty));
            resultVec.push_back(sBlock);
            // and cache it
            biggestBlockPosition = cfgfile.tellg();
            blockPositionCache[sBlock] = biggestBlockPosition;
        }
    }

    if(!cfgfile.eof())
        throw std::runtime_error("Error reading from config file! Filename: " + sFilename);

    cfgfile.clear();
    cfgfile.seekg(0, cfgfile.beg);
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
        cutAfterFirst(sLine, "#", "\\");
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
            logWARNING("CfgFile") << "Syntax error in configuration file! File: " << sFilename <<" Block: " << sBlock << " Key: " << sLine;
            continue; // syntax error, we ignore the block (comment or end of line)
        }

        // get the value
        sValue = sLine;
        if (getKeyValue(sValue, endOfName) != 0)
        {
            logWARNING("CfgFile") << "Syntax error in configuration file! File: " << sFilename << " Block: " << sBlock <<
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
        cfgfile.seekg(0, cfgfile.beg);
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
    configList resultVec(blockPositionCache.size());
    blockMap tmpMap;

    // go to the start
    cfgfile.seekg(0, cfgfile.beg);

    // and parse the whole file
    while(cfgfile.good())
    {
        getline(cfgfile,sLine);
        cutAfterFirst(sLine, "#", "\\");

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
                logWARNING("CfgFile") << "Syntax error in configuration file! File: " << sFilename <<" Block: " << sLine;
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
            biggestBlockPosition = cfgfile.tellg();
            blockPositionCache[sThisBlocksName] = biggestBlockPosition;
        }
        else if( !sThisBlocksName.empty()) // a key (ignore keys before first block)
        {
            // add the key to tmpMap

            // find the key name
            endOfName = sLine.find_first_of(" =\t", nonEmpty+1);
            if(endOfName == std::string::npos)
            {
                logWARNING("CfgFile") << "Syntax error in configuration file! File: " << sFilename <<" Block: " << sThisBlocksName << " Key: " << sLine;
                continue; // syntax error, we ignore the key
            }

            // get the value
            sValue = sLine;
            if (getKeyValue(sValue, endOfName) != 0)
            {
                logWARNING("CfgFile") << "Syntax error in configuration file! File: " << sFilename << " Block: " <<
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
    cfgfile.seekg(cfgfile.beg);
    sCurrentBlock = ""; // we are at 0, so in no block

    return resultVec;
}

void CfgFile::addComment(const std::string &s)
{
    if (s.find_first_of("\n\r") != std::string::npos)
        throw std::invalid_argument("I can't write a string containing a 'new line' to the config file!");

    cfgfile.seekg(0, cfgfile.end);
    sCurrentBlock = "";

    cfgfile.unget();
    if (cfgfile.get() != '\n')
        cfgfile << '\n';
    cfgfile.clear();

    if (s[s.find_first_not_of(" \t")] != '#')
        cfgfile << "# ";

    cfgfile << s << "\n";
    cfgfile.flush();
}

void CfgFile::addBlock(const std::string &sBlock, const blockMap &block)
{
    if (findBlock(sBlock) == 0)
    {
        std::ofstream f;
        copyFirstPart(f, cfgfile.tellg());
        for (auto item : block)
        {
            std::string sValue = item.second;

            if (sValue.find_first_of("\n\r") != std::string::npos)
                throw std::invalid_argument("I can't write a string containing a 'new line' to the config file!");

            escapeString(sValue, "\"#", '\\');
            if (sValue.find_first_of(" \t") != std::string::npos)
                sValue = "\"" + sValue + "\"";

            f << '\t' << item.first << " = " << sValue << "\n";
        }
        findKey("");
        copySecondPart(f, cfgfile.tellg());
    }
    else
    {
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
        for (auto item : block)
        {
            std::string sValue = item.second;

            if (sValue.find_first_of("\n\r") != std::string::npos)
                throw std::invalid_argument("I can't write a string containing a 'new line' to the config file!");

            escapeString(sValue, "\"#", '\\');
            if (sValue.find_first_of(" \t") != std::string::npos)
                sValue = "\"" + sValue + "\"";

            cfgfile << '\t' << item.first << " = " << sValue << "\n";
        }
        cfgfile.flush();
    }
}

void CfgFile::removeBlock(const std::string &sBlock)
{
    if (findBlock(sBlock) != 0)
        throw std::invalid_argument("The Block " + sBlock + " was not found in the config file " + sFilename);

    std::ofstream f;
    copyFirstPart(f, cfgfile.tellg());
    findKey("");
    copySecondPart(f, cfgfile.tellg());
}

void CfgFile::removeKey(const std::string &sBlock, const std::string &sKey)
{
    if (findBlock(sBlock) != 0)
        throw std::invalid_argument("The Block " + sBlock + " was not found in the config file " + sFilename);
    if (findKey(sKey) != 0)
        throw std::invalid_argument(
                "The Key " + sKey + " in Block " + sBlock + " was not found in the config file " + sFilename);

    std::ofstream f;
    copyFirstPart(f, cfgfile.tellg());
    cfgfile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    copySecondPart(f, cfgfile.tellg());
}

int CfgFile::findBlock(const std::string &sBlock)
{
    // see if its already in the cache
    if (blockPositionCache.count(sBlock) > 0)
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

    // we need to search manually start at the block in the cache with the highest position
    // or the beginning if the cach is empty
    if (cfgfile.eof())
        cfgfile.clear();
    cfgfile.seekg(biggestBlockPosition);

    // then search to eof
    while (cfgfile.good())
    {
        getline(cfgfile, sLine);

        // the first non whitespace char
        nonEmpty = sLine.find_first_not_of(" \t");

        // ignore white lines
        if (nonEmpty == std::string::npos)
            continue;

        // now check if the current line is a block
        if (sLine[nonEmpty] == '[')
        {
            // find the closing bracket
            closingBracket = sLine.find_first_of("]# ");
            if (sLine[closingBracket] != ']')
            {
                logWARNING("CfgFile") << "Syntax error in configuration file! File: " << sFilename << " Block: " << sLine;
                continue; // syntax error, we ignore the block (comment or end of line)
            }

            // now check if what we got is what we are looking for
            nonEmpty++; // ignore the [
            if (sLine.substr(nonEmpty, closingBracket - (nonEmpty)) == sBlock)
            {
                biggestBlockPosition = cfgfile.tellg(); //every smaller value is definitely cached
                blockPositionCache[sBlock] = biggestBlockPosition;
                sCurrentBlock = sBlock;
                return 0; // we found it
            }
            else // cache the position
            {
                biggestBlockPosition = cfgfile.tellg(); //every smaller value is definitely cached
                blockPositionCache[sLine.substr(nonEmpty, closingBracket - (nonEmpty))] = biggestBlockPosition;
            }
        }
    }

    if (!cfgfile.eof()) // for eof go back to the start
        throw std::runtime_error("Error reading from config file! Filename: " + sFilename);

    cfgfile.clear();
    cfgfile.seekg(0, cfgfile.beg);
    sCurrentBlock = "";

    // didnt find anything
    return -1;
}

int CfgFile::getKeyValue(std::string &sLine, const size_t startPos)
{
    // we found the key now get the value
    size_t valueBegin = sLine.find_first_not_of(" =\t", startPos);
    if (valueBegin == std::string::npos)
        return 1; // syntax error, we ignore the block (comment or end of line)

    size_t valueEnd;
    if (sLine[valueBegin] == '\"')
    {
        // find a second '"' which is not escaped
        valueBegin++;
        valueEnd = findFirstNotEscapedOf(sLine, "\"", valueBegin);

        if (valueEnd == std::string::npos)
            return 1; // syntax error, we ignore the block (comment or end of line)

        sLine = sLine.substr(valueBegin, valueEnd - valueBegin);
        unescapeString(sLine);
    }
    else
    {
        valueEnd = sLine.find_last_not_of(" \t");
        if (valueEnd == std::string::npos)
            return 1; // syntax error, we ignore the block (comment or end of line)

        valueEnd++;
        sLine = sLine.substr(valueBegin, valueEnd - valueBegin);
        removeWhite(sLine);
        unescapeString(sLine);
    }

    return 0;
}

int CfgFile::findKey(const std::string &sKey)
{
    // we only search from here to the end of the Block

    // some local variables
    std::string sLine;
    size_t nonEmpty;
    size_t endOfName;
    std::streampos beforeLine; // this is important, we need to be able to reset the file readposition
    // to right after th previous non empty line

    // save where we are
    beforeLine = cfgfile.tellg();

    // look at all keys to the next block or to eof
    while (cfgfile.good())
    {
        // get the next line
        getline(cfgfile, sLine);

        // the first non whitespace char
        nonEmpty = sLine.find_first_not_of(" \t");

        if (nonEmpty == std::string::npos)
            continue; // ignore empty lines (and dont update beforeLine)
        if (sLine[nonEmpty] == '#')
        {
            beforeLine = cfgfile.tellg();
            continue;
        }

        if (sLine[nonEmpty] == '[')
        {
            // we are done this is the next block
            cfgfile.seekg(beforeLine);
            return 1;
        }

        // we found a key
        cutAfterFirst(sLine, "#", "\\");
        // see where the name is
        endOfName = sLine.find_first_of(" =\t", nonEmpty + 1);
        if (endOfName == std::string::npos)
        {
            logWARNING("CfgFile") << "Syntax error in configuration file! File: " << sFilename << " Key: " << sLine;
            continue; // syntax error, we ignore the line
        }

        // check if it is our key
        if (sLine.substr(nonEmpty, endOfName - (nonEmpty)) == sKey)
        {
            // we found our key, beforeLine is set to the end of the line before (ignoring empty ones) so set this as the new position
            cfgfile.seekg(beforeLine);
            return 0;
        }

        // this was a key, but not the one we are looking for => update beforeLine
        beforeLine = cfgfile.tellg();
    }

    if (!cfgfile.eof())
        throw std::runtime_error("Error reading from config file! Filename: " + sFilename);

    // we are at the end of the file, reset the position to the last saved position
    // which is right behind the ’\n’ char of the last line in the file which is not
    // empty or comment ony (because we only saved our position after finding a key)
    cfgfile.clear();
    cfgfile.seekg(beforeLine);
    return 2;
}

void CfgFile::copyFirstPart(std::ofstream &filestream, const std::streampos &to)
{
    filestream.open(sFilename + ".tmp", std::ofstream::out);
    if (!filestream.is_open())
        throw std::runtime_error("Could not open temp config file: " + sFilename + ".tmp");

    char *buff = new char[to];
    cfgfile.seekg(cfgfile.beg);
    sCurrentBlock = "";
    cfgfile.read(buff, to);
    filestream.write(buff, to);

    MPU_SAVE_DELETE(buff)
}

void CfgFile::copySecondPart(std::ofstream &filestream, const std::streampos &from)
{
    cfgfile.seekg(0, cfgfile.end);
    std::streampos e = cfgfile.tellg();
    size_t l = e - from;

    cfgfile.seekg(from);
    char *buff = new char[l];
    cfgfile.read(buff, l);
    filestream.write(buff, l);

    MPU_SAVE_DELETE(buff);

    filestream.close();
    close();

    if (rename((sFilename + ".tmp").c_str(), sFilename.c_str()) != 0)
        throw std::runtime_error("Error renaming the temp File to " + sFilename + "Errno: " + strerror(errno));

    open(sFilename);
}

}