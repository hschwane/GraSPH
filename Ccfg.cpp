/*
 * libTars
 * Ccfg.cpp
 *
 * @author: Hendrik Schwanekamp
 *
 * Implementiert die Klasse Ccfg, die funktionen zum verwalten einer config datei bereit stellt.
 *
 */

// Includes
//--------------------
#include "Ccfg.h"
#include "libTars.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <cerrno>
#include <string>
#include <sstream>
//--------------------

// Funktionen der CLog Klasse
//-------------------------------------------------------------------

/**
 * Cunstructor
 */
Ccfg::Ccfg()
{
    pCfgFile = NULL;
    sDescription = "";
    sCfgFilePath = "";
    sName = "";
    iLineLenght = 0;
    fVersion = 0;
    bIsInit = false;
    bIsLoaded = false;
}

/**
 * Destructor
 */
Ccfg::~Ccfg()
{
    if( isInit())
    {
        std::cerr << "Forget to exit cfg class. Exiting... AT:" + FILEPOS + "\n";
        exit();
    }
}

/**
 * Initialisieren die cfg Klasse mit dem Programmnamen als Name. Ruft load() mit auf.
 *
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklppt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::init()
{
    if( !LIBTARS_READY)
    {
        std::cerr << "Trying to init config file before library! Call libTarsInit() first. at: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }
    return init( pTarsLib->getAppName());
}

/**
 * Initialisieren der config Datei mit einem angegebenem Namen. Ruft load() mit auf.
 *
 * @param sName: Der Name der config Datei, ohne pfad und endung als std::string.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklppt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::init(std::string sName)
{
    if( !LIBTARS_READY)
    {
        std::cerr << "Trying to init config file before library! Call libTarsInit() first. at: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    if( isInit())
    {
        WARNING("The Ccfg is already initalised! Call reinit instead.");
        return RT_ERROR;
    }

    INFO("Initalising a config file... ");

    // parameter prüfen
    if(!isDirectory(pTarsLib->getPathToRoot()+"cfg/"))
    {
        if( createDirectory("cfg/") != 0)
        {
            ERROR("No Permission to create cfg Directory " + pTarsLib->getPathToRoot() + "cfg/");
            return RT_NO_PERMISSION;
        }
    }

    // member setzen
    sCfgFilePath = pTarsLib->getPathToRoot() + "cfg/" + sName + ".cfg";
    bIsInit = true;
    iLineLenght = 255;
    this->sName = sName;
    iterCurrBlockName = "NONE";

    // lokale variablen
    int iResult;
    char cLine[iLineLenght];
    char cBuff[iLineLenght];

    // laden
    iResult = load();
    if( iResult != 0)
    {
        bIsInit = false;
        ERROR("Error loading cfg file. Calling load() return code was " + rtToString(iResult));
        unload();
        bIsInit = false;
        return iResult;
    }

    // überprüfen ob die datei eine TarsCfg Datei ist und lesen des Headers
    if( fgets(cLine, 15, pCfgFile) == NULL)
    {
        ERROR("Error reading config File, unexpected end of File, or read Error!");
        unload();
        bIsInit = false;
        return RT_FILE_ERROR;
    }
    if( strcmp(cLine, "<--libTarsCfg\n") != 0)
    {
        ERROR("Error reading config File, header is incorrect!");
        unload();
        bIsInit = false;
        return RT_FILE_ERROR;
    }

    // jetzt den header laden
    while( true)
    {
        if( fgets(cLine, iLineLenght, pCfgFile) == NULL)
        {
            ERROR("Error reading config File, unexpected end of File, or read Error!");
            unload();
            bIsInit = false;
            return RT_FILE_ERROR;
        }

        prepareLine(cLine);
        if( strcmp(cLine, "-->") == 0)
            break; // header ist zuende
        else if(cLine[0] != '\0')
        {
            iResult = nameFromKey( cLine, cBuff);
            if( iResult != 0)
            {
                ERROR("Error reading name of Key from the config file, File is probably damaged. Called nameFromKey() return code was " + rtToString(iResult));
                return iResult;
            }

            // welchen Headerkey haben wir gefunden?
            if( strcmp(cBuff, "maxLLenght") == 0)
            {
                // die maximale zeilenlänge
                iResult = valueFromKey( cLine, cBuff);
                if( iResult != 0)
                {
                    ERROR("Error reading value of Key from the config file, File is probably damaged. Called valueFromKey() return code was " + rtToString(iResult));
                    return iResult;
                }
                std::stringstream ss(cBuff);
                if ( !(ss >> iLineLenght))
                {
                    ERROR("Error reading value of Key from the config file, File is probably damaged. maxLLenght was of wrong type.");
                    return RT_WRONG_TYPE;
                }
            }
            else if( strcmp(cBuff, "desc") == 0)
            {
                // die beschreibung
                iResult = valueFromKey( cLine, cBuff);
                if( iResult != 0)
                {
                    ERROR("Error reading value of Key from the config file, File is probably damaged. Called valueFromKey() return code was " + rtToString(iResult));
                    return iResult;
                }
                sDescription = cBuff;
            }
            else if( strcmp(cBuff, "version") == 0)
            {
                // die version
                iResult = valueFromKey( cLine, cBuff);
                if( iResult != 0)
                {
                    ERROR("Error reading value of Key from the config file, File is probably damaged. Called valueFromKey() return code was " + rtToString(iResult));
                    return iResult;
                }
                std::stringstream ss(cBuff);
                if ( !(ss >> fVersion))
                {
                    ERROR("Error reading value of Key from the config file, File is probably damaged. version was of wrong type.");
                    return RT_WRONG_TYPE;
                }
            }
            // alles andere interessiert uns hier nicht und wird einfach ignoriert
        }
    }

    fgetpos (pCfgFile,&fPosBody); // zum zurück nach oben springen.
    fPosIter = fPosBody; // den Iterator setzen.

    EVENT("Initalisation of config file complete.");
    return RT_OK;
}

/**
 * Verlässt die config Datei. Ruft wenn nötig unload auf.
 *
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklppt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::exit()
{
    if( !isInit())
    {
        if(LIBTARS_READY)
        {
            WARNING("Trying to exit config file, but config file is not even initalised!");
        }
        else
            std::cerr << "Trying to exit config file, but config file is not even initalised! AT: " + FILEPOS + "\n";
        return RT_NOT_INITED;
    }

    INFO("Exiting config file...")

    if(isLoaded())
    {
        int iResult = unload();
        if( iResult != 0)
        {
            bIsInit = false;
            ERROR("Error unloading config file. Called unload(), return code was: " + rtToString(iResult));
            return iResult;
        }
    }

    bIsInit = false;
    EVENT("Exited config file.");
    return RT_OK;
}

/**
 * Läd die config-datei. Wird von init() mit aufgerufen. Öffnet die config-datei (den Dateihandle).
 *
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklppt hat, RT_NO_FILE wenn die init datei nicht existiert. Siehe libTars.h für mehr infos.
 */
int Ccfg::load()
{
    if( !isInit())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to load config file, but config file is not even initalised!");
        }
        else
            std::cerr << "Trying to load config file, but config file is not even initalised! AT: " + FILEPOS + "\n";
        return RT_NOT_INITED;
    }

    if( isLoaded())
    {
        WARNING("The config file is already loaded! Call reload instead.");
        return RT_ERROR;
    }

    INFO("Loading config file...");

    pCfgFile = fopen(sCfgFilePath.c_str(), "r");

    if(pCfgFile == NULL)
    {
        // check errno
        if(errno == ENOENT)
        {
            ERROR("Config file does not exist");
            return RT_NO_FILE;
        }

        ERROR("Error opening config file with fopen. ERRNO=" + errno);
        return RT_FILE_ERROR;
    }

    bIsLoaded = true;
    INFO("Config file loaded.");
    return RT_OK;
}

/**
 * Entläd die cfg-datei. Wird von exit() mit aufgerufen.
 *
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklppt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::unload()
{
    if( !isLoaded())
    {
        WARNING("Trying to unload cfg-file, which is not even loaded!")
        return RT_NOT_LOADED;
    }

    INFO("Unloading config file...");
    fclose( pCfgFile);

    bIsLoaded = false;
    INFO("Config file unloaded.");
    return RT_OK;
}

/**
 * Neuinitialisieren der cfg Datei mit dem Programmnamen als Namen
 *
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklppt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::reinit()
{
    if( !LIBTARS_READY)
    {
        std::cerr << "Trying to init config file before library! Call libTarsInit() first. at: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }
    return reinit( pTarsLib->getAppName());
}

/**
 * Neunitialisieren der cfg Datei mit einem angegebenem Namen
 *
 * @param sName: Der Name der cfg Datei als std::string, wird der Dateiname und die Überschrift.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklppt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::reinit(std::string sName)
{
    if( !isInit())
    {
        if(LIBTARS_READY)
        {
            WARNING("Trying to reinit config file, but config file is not even initalised! Calling init instead...");
            return init( sName);
        }
        else
            std::cerr << "Trying to reinit config file, but config file is not even initalised! And TarsLib is not ready as well!  AT: " + FILEPOS + "\n";
        return RT_NOT_INITED;
    }

    INFO("Reiniting config file...");

    // lokale variablen
    int iResult;

    // verlassen
    iResult = exit();
    if( iResult != 0)
    {
        ERROR("Error Exiting config file while reinit. Called exit(), return code was: " + rtToString(iResult));
        return iResult;
    }

    // initialisieren
    iResult = init( sName);
    if( iResult != 0)
    {
        ERROR("Error Initalising config file while reinit. Called init(), return code was: " + rtToString(iResult));
        return iResult;
    }

    INFO("Reinit complete.");
    return RT_OK;
}

/**
 * Erstellen einer neuen cfg Datei und initialisieren der Klasse, mit dem Programmnamen als Name.
 * Einer nichtssagenden standardbeschreibung und der Version 1.0.
 *
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklppt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::createNew()
{
    if( !LIBTARS_READY)
    {
        std::cerr << "Trying to create new config file before initalising the library! Call libTarsInit() first. at: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    return createNew(pTarsLib->getAppName(), pTarsLib->getAppName() + " (Applications Name) main config file.", 1.0);
}

/**
 * Erstellen einer neuen cfg Datei und initialisieren der Klasse mit den Angegebenen Werten.
 *
 * @param sName: Der Name der Config Datei als std::string;
 * @param sDescription: Die Beschreibung der Config Datei als std::string;
 * @param fVersion: Die Version der Config Datei als float;
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklppt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::createNew(std::string sName, std::string sDescription, float fVersion)
{
    if( !LIBTARS_READY)
    {
        std::cerr << "Trying to create new config file before initalising the library! Call libTarsInit() first. at: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    INFO("Creating a new config file... ");

    // parameter prüfen
    if(!isDirectory(pTarsLib->getPathToRoot()+"cfg/"))
    {
        if( createDirectory("cfg/") != 0)
        {
            ERROR("No Permission to create cfg Directory " + pTarsLib->getPathToRoot() + "cfg/");
            return RT_NO_PERMISSION;
        }
    }

    // verlassen wenn initialisiert
    if( isInit())
        exit();

    // die neue Datei erstellen
    sCfgFilePath = pTarsLib->getPathToRoot() + "cfg/" + sName + ".cfg";
    pCfgFile = fopen(sCfgFilePath.c_str(), "w");
    if(pCfgFile == NULL)
    {
        ERROR("Error opening config file with fopen. ERRNO=" + errno);
        return RT_FILE_ERROR;
    }

    // den header schreiben
    iLineLenght = sDescription.length() + 18;
    if( fprintf( pCfgFile, "<--libTarsCfg\n    maxLLenght=%i\n    desc=\"%s\"\n    version=%f\n-->" , iLineLenght, sDescription.c_str(), fVersion) < 0)
    {
        fclose(pCfgFile);
        ERROR("Fehler beim schreiben in die neue cfg Datei.");
        return RT_FILE_ERROR;
    }

    fclose(pCfgFile);
    EVENT("Config file creation completed, reiniting the config file class now...");

    int iResult = init(sName);
    if( iResult != 0)
        ERROR("Error initalising a config file, after creating  new config file! Called init(), return code was: " + rtToString(iResult));
    return iResult;
}

/**
 * Vorbereiten einer eingelesenen Zeile für die analyse. Leerzeichen, Kommentare, usw. werden entfernt.
 *
 * @param line: ein pointer auf den c-string der die eingelesene Zeile enthält.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklppt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::prepareLine(char* line)
{
    // Parameter checken
    if(line == NULL)
        return RT_ARG_INV;

    // lokale variablen
    char* p_ci = NULL;       // schreibzähler
    char* p_cj = line;       // lesezähler
    bool bRm = true;         // remove spaces
    bool bEscape = false;    // next char is escaped

    // '\n' mit '\0' überschreiben wenn es existiert
    size_t p=strlen(line);
    if(line[p-1] == '\n')
        line[p-1]='\0';

    // search for a comment and end the line there if found
    p_ci = strchr( line,'#');
    if( p_ci != NULL)
        *p_ci = '\0';

    // remove all whitespace that is not in parentheses ignores escaped parentheses
    p_ci = line;
    while(*p_cj != '\0')
    {
        *p_ci = *p_cj++;

        if( *p_ci == '\"' && !bEscape)
            bRm = !bRm;

        if( *p_ci == '\\')
            bEscape = true;
        else
            bEscape = false;

        if( !((isspace(*p_ci) && bRm)))
            p_ci++;
    }
    *p_ci = '\0';

    return RT_OK;
}

/**
 * Lesen eines Blocknamens aus einer Zeile. Wenn die Zeile keinen Blocknamen enthält wird RT_U_LINE_END returned.
 *
 * @param line: ein pointer auf den c-string der die eingelesene Zeile enthält, nicht NULL.
 * @param blockName: ein pointer auf einen c-string, hier wird der Block-Name gespeichert, nicht NULL.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklppt hat, RT_U_LINE_END wenn die Zeile keienen Blocknamen enthält. Siehe libTars.h für mehr infos.
 */
int Ccfg::nameFromBlock(const char* line, char* blockName)
{
    // parameter prüfen
    if(line == NULL || blockName == NULL)
        return RT_ARG_INV;

    // checken ob es ein Blockname sein kann
    if(line[0] != '[')
        return RT_U_LINE_END; // unexpected end of line

    // lokale variablen
    const char* cBuff;
    int lengthH;

    // schließende Klammer suchen
    cBuff = strchr( line,']');
    if( cBuff == NULL)
        return RT_U_LINE_END; // unexpected end of line

    // namen kopieren
    lengthH = cBuff-(line+1);
    memcpy(blockName, line+1, lengthH);
    blockName[lengthH] = '\0';

    return RT_OK;
}

/**
 * Lesen eines Keynamens aus einer Zeile. Wenn die Zeile keinen Key enthält wird RT_U_LINE_END returned.
 *
 * @param line: ein pointer auf den c-string der die eingelesene Zeile enthält, nicht NULL.
 * @param keyName: ein pointer auf einen c-string, hier wird der Key-Name gespeichert, nicht NULL.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklppt hat, RT_U_LINE_END wenn die Zeile keienen Key enthält. Siehe libTars.h für mehr infos.
 */
int Ccfg::nameFromKey(const char *line, char* keyName)
{
    // parameter prüfen
    if(line == NULL || keyName == NULL)
        return RT_ARG_INV;

    // lokale variablen
    const char* cBuff;
    int lengthH;

    // gleich zeichen suchen
    cBuff = strchr( line,'=');
    if( cBuff == NULL)
        return RT_U_LINE_END; // unexpected end of line

    // namen kopieren
    lengthH = cBuff-line;
    memcpy(keyName, line, lengthH);
    keyName[lengthH] = '\0';

    return RT_OK;
}

/**
 * Lesen eines Keyvalues aus einer Zeile. Wenn die Zeile keinen Key enthält, oder dieser ungültig ist, wird RT_U_LINE_END returned.
 *
 * @param line: ein pointer auf den c-string der die eingelesene Zeile enthält, nicht NULL.
 * @param keyName: ein pointer auf einen c-string, hier wird der Key-Value gespeichert, nicht NULL.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklppt hat, RT_U_LINE_END wenn die Zeile keienen Key enthält. Siehe libTars.h für mehr infos.
 */
int Ccfg::valueFromKey(const char* line, char* keyValue)
{
    // parameter prüfen
    if(line == NULL || keyValue == NULL)
        return RT_ARG_INV;

    // lokale variablen
    const char *start;
    const char *first;
    const char *second;
    int lengthK;

    // erstes gleich ('=') suchen
    start=strchr( line,'=');

    if(start == NULL)
        return RT_U_LINE_END; // unexpected end of line

    start++;

    // nach einem nicht escapten " suchen
    first=strchr( start,'\"');
    while (first!=NULL)
    {
        if( *(first-1) != '\\')
            break;
        else
        {
            const char* i = first-1;
            int num = 0;
            while( *i == '\\')
            {
                i--;
                num++;
            }

            if(num % 2 == 0)
                break;
        }

        second=strchr(first+1,'\"');
    }

    if(first == NULL) // keins gefunden, alles von gleich bis zum ende mitnehmen
    {
        strcpy(keyValue, start);
    }
    else // anführungszeichen gefunden, zweites suchen und alles dazwischen mitnehmen
    {
        second = strchr( first+1,'\"');
        while (second!=NULL)
        {
            if( *(second-1) != '\\')
                break;
            else
            {
                const char* i = second-1;
                int num = 0;
                while( *i == '\\')
                {
                    i--;
                    num++;
                }

                if(num % 2 == 0)
                    break;
            }

            second=strchr(second+1,'\"');
        }

        if(second == NULL)
            return RT_U_LINE_END; // unexpected end of line

        lengthK = second - (first+1);
        memcpy(keyValue, first+1, lengthK);
        keyValue[lengthK] = '\0';
    }

    // remove all single backspace
    char* i = keyValue;
    char* j = keyValue;
    bool rm = true;

    while(*j != '\0')
    {
        *i = *j++;

        if( !(*i=='\\') || rm == false)
        {
            i++;
            rm = true;
        }
        else
            rm = false;
    }
    *i = '\0';

    return RT_OK;
}

/**
 * Suchen eines Blocks und setzen des Dateicoursers an die entsprechende Stelle. Achtung: die Suche beginnt an der aktuellen Dateiposition.
 *
 * @param blockName: ein pointer auf den c-string der den Namen des gesuchten Blocks enthält, nicht NULL.
 * @param pCopy: Wenn ein gültiger Dateizeiger übergeben wurde werden alle Zeilen unterwegs hierher kopiert.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::gotoBlock(const char* blockName, FILE* pCopy)
{
    // parameter prüfen
    if( blockName == NULL)
        return RT_ARG_INV;

    // lokale variablen
    char cLine[iLineLenght];
    char cBuff[iLineLenght];
    int iResult = 0;

    // den block suchen
    while( true)
    {
        if( fgets(cLine, iLineLenght, pCfgFile) == NULL)
        {
            if(feof(pCfgFile) != 0)
                return RT_NOT_FOUND;
            if(ferror(pCfgFile) != 0)
                return RT_FILE_ERROR;
        }

        if(pCopy != NULL)
        {
            if( fputs( cLine, pCopy) < 0)
            {
                ERROR("Error writing the Line to the pCopy file.");
                return RT_FILE_ERROR;
            }
        }

        prepareLine(cLine);
        if(cLine[0] == '[')
        {
            // es ist ein Block, Namen überprüfen
            iResult = nameFromBlock( cLine, cBuff);
            if( iResult != 0)
                WARNING("Error reading name of Block from the config file, File is probably damaged. Called nameFromBlock() return code was " + rtToString(iResult));

            if( strcmp( cBuff, blockName) == 0)
                 return RT_OK; // wir sind jetzt im richtigen Block
        }
    }
    return iResult; // wird nie erreicht
}

/**
 * Suchen eines Keys und setzen des Dateicoursers HINTER diesen Key. Die Zeile die den Gesuchten Key Enthält wird in lineOut geschrieben.
 * Wird der Key innerhalb des aktuellen Blocks nicht gefunden wird RT_NOT_FOUND zurück gegeben.
 * Achtung: die Suche beginnt an der aktuellen Dateiposition.
 *
 * @param keyName: ein pointer auf den c-string der den Namen des gesuchten Keys enthält, nicht NULL.
 * @param lineOut: ein pointer auf einen c-string, hierher wird die Zeile geschrieben, die den gesuchten Key enthält, nicht NULL.
 * @param pCopy: Wenn ein gültiger Dateizeiger übergeben wurde werden alle Zeilen unterwegs hierher kopiert, den Key selbst aber nicht.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::gotoKey(const char* keyName, char* lineOut, FILE* pCopy)
{
    // parameter prüfen
    if( keyName == NULL || lineOut == NULL)
        return RT_ARG_INV;

    // lokale variablen
    char cBuff[iLineLenght];
    char cOrLine[iLineLenght];
    fpos_t fPosBlockEnd;
    fpos_t fPosCpyBlockEnd;

    if(pCopy != NULL)
        fgetpos(pCopy,&fPosCpyBlockEnd);

    while( true)
    {
        fgetpos(pCfgFile,&fPosBlockEnd);

        if( fgets(lineOut, iLineLenght, pCfgFile) == NULL)
        {
            if(feof(pCfgFile) != 0)
            {
                if(pCopy != NULL)
                {
                    if(cOrLine[0] == '\n')
                        fsetpos(pCopy,&fPosCpyBlockEnd);
                }
                return RT_NOT_FOUND;
            }
            if(ferror(pCfgFile) != 0)
                return RT_FILE_ERROR;
        }

        if(pCopy != NULL)
            strcpy( cOrLine, lineOut);

        prepareLine(lineOut);
        if(lineOut[0] == '[')
        {
            fsetpos(pCfgFile,&fPosBlockEnd);
            if(pCopy != NULL)
                fsetpos(pCopy,&fPosCpyBlockEnd);
            return RT_NOT_FOUND; // ende des Blocks
        }
        else if(lineOut[0] != '\0')
        {
            nameFromKey( lineOut, cBuff);
            if( strcmp( cBuff, keyName) == 0)
                return RT_OK; // wir haben den Key gefunden
        }

        if(pCopy != NULL)
        {
            fgetpos(pCopy,&fPosCpyBlockEnd);
            if( fputs( cOrLine, pCopy) < 0)
            {
                ERROR("Error writing the Line to the pCopy file.");
                return RT_FILE_ERROR;
            }
        }
    }
    return RT_OK; // wird nie erreicht
}

/**
 * Schreiben eines Keys in einem bestimmten Block mit einem Integer-Wert. Wenn der Key oder der Block nicht existiert wird er neu erstellt.
 * Wenn für value NULL übergeben wird, wird der Key gelöscht.
 *
 * @param block: der Name des Blocks als pointer auf einen cString.
 * @param key: der Name des Keys als pointer auf einen cString.
 * @param value: Der Wert des neuen Keys als pointer auf einen cString, wenn NULL wird der Key gelöscht.
 * @param header: Wenn true wird der Key im Header hinzugefügt
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::addValueRaw( const char* block, const char* key, const char* value, bool header)
{
    //parameter prüfen
    if( block == NULL || key == NULL)
        return RT_ARG_INV;

    // lokale variablen
    char cBuff[iLineLenght];
    int iResult = 0;
    FILE* pTempCfg = NULL;;
    std::string sTempCfgPath = pTarsLib->getPathToRoot() + "cfg/" + sName + "_new_temp" + ".cfg";

    // temp datei öffnen
    pTempCfg = fopen(sTempCfgPath.c_str(), "w");
    if(pTempCfg == NULL)
    {
        ERROR("Error opening temp config file with fopen for changeing. ERRNO=" + errno);
        return RT_FILE_ERROR;
    }

    // reset file courser to the start
    rewind(pCfgFile);

    if( header == false)
    {
        // den Block suchen
        iResult = gotoBlock( block, pTempCfg);
        if( iResult != 0 && iResult != RT_NOT_FOUND)
        {
            ERROR("Error searching for Block in config file, Called gotoBlock(), return code was " + rtToString(iResult));
            fclose(pTempCfg);
            return iResult;
        }
    }

    if( iResult == RT_NOT_FOUND)
    {
        // wenn wir nur löschen wollen sind wir fertig, wenn der block nicht existiert
        if( value == NULL)
            return RT_OK;

        // block neu anlegen am dateiende ( wo wir jetzt eh sind)
        if( fprintf( pTempCfg, "\n[%s]\n" , block) < 0)
        {
            fclose(pTempCfg);
            ERROR("Fehler beim neu anlegen eines Blocks in der cfg Datei.");
            return RT_FILE_ERROR;
        }
    }
    else
    {
        // den Key suchen
        iResult = gotoKey( key, cBuff, pTempCfg);
        if( iResult != 0 && iResult != RT_NOT_FOUND)
        {
            ERROR("Error searching for Key in config file, Called gotoKey(), return code was " + rtToString(iResult));
            fclose(pTempCfg);
            return iResult;
        }
    }

    // neuen key anlegen, außer wir sollen ihn löschen (wenn value != NULL ist)
    if( value != NULL)
    {

        // key schreiben
        // am blockende in der Datei ein zusätzliches leerzeichen
        if(iResult == RT_NOT_FOUND && feof(pCfgFile) == 0)
            iResult = fprintf( pTempCfg, "    %s = %s\n\n" , key, value);
        else
            iResult = fprintf( pTempCfg, "    %s = %s\n" , key, value);

        if( iResult < 0)
        {
            fclose(pTempCfg);
            ERROR("Fehler beim neu anlegen eines Keys in der cfg Datei.");
            return RT_FILE_ERROR;
        }
    }

    // den rest der Datei kopieren
    while( true)
    {
        if( fgets( cBuff, iLineLenght, pCfgFile) == NULL)
        {
            if( feof(pCfgFile) != 0)
            {
                break;
            }
            else
            {
                fclose(pTempCfg);
                ERROR("Fehler beim lesen des Rests aus der alten Cfg Datei.");
                return RT_FILE_ERROR;
            }
        }
        if( fputs( cBuff, pTempCfg)  < 0)
        {
            fclose(pTempCfg);
            ERROR("Fehler beim schreiben des Rests in die neue Cfg Datei.");
            return RT_FILE_ERROR;
        }
    }

    // die alte datei mit der neuen überschreiben
    fclose(pTempCfg);
    iResult = unload();
    if( iResult != 0)
    {
        ERROR("Error unloading config file. Called unload(), return code was: " + rtToString(iResult));
        return iResult;
    }

    if( remove( sCfgFilePath.c_str()) != 0)
    {
        ERROR("Error removeing old config file.");
        return RT_FILE_ERROR;
    }
    if( rename( sTempCfgPath.c_str(), sCfgFilePath.c_str()) != 0)
    {
        ERROR("Error renaming temp config file.");
        return RT_FILE_ERROR;
    }

    iResult = load();
    if( iResult != 0)
    {
        ERROR("Error loading config file. Called load(), return code was: " + rtToString(iResult));
        return iResult;
    }

    return RT_OK;
}

/**
* Getter für die vermutlich Maximale Zeilenlänge in der Datei, diese länge wird für alle Puffer verwendet.
* Wenn eine Zeile nicht ganz gelesen werden konnte wird diese Länge erhöht.
*
* @return die vermutlich Maximale Zeilenlänge in der Datei als integer.
*/
int Ccfg::getLineLenght()
{
    return iLineLenght;
}

/**
* Getter für die Dateiversion.
*
* @return die Dateiversion als float.
*/
float Ccfg::getVersion()
{
    return fVersion;
}

/**
* Getter für die Beschreibung der Config Datei.
*
* @return die Beschreibung der Config Datei als std::string.
*/
std::string Ccfg::getDescription()
{
    return sDescription;
}

/**
 * Updaten der Beschreibung der Config Datei.
 *
 * @param description: Die neue Beschreibung als std::string.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::updateDescription(std::string description)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to update a Header Value in a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to update a Header Value in a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    int iResult = addValueRaw("NONE", "desc", std::string("\"" + description + "\"").c_str(), true);
    if(iResult != 0)
        return iResult;

    sDescription = description;
    return RT_OK;
}

/**
 * Updaten der Version der Config Datei.
 *
 * @param newVersion: Die neue Version als float.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::updateVersion(float newVersion)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to get a Value from a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to get a Value from a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    int iResult = addValueRaw("NONE", "version", std::to_string(newVersion).c_str(), true);
    if(iResult != 0)
        return iResult;

    fVersion = newVersion;
    return RT_OK;
}

/**
 * Updaten der Vermutlichen Maximalen Zeilenlänge.
 *
 * @param newLineLenght: Die neue vermutliche Maximalen Zeilenlänge als int.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::updateLineLenght(int newLineLenght)
{
    int iResult = addValueRaw("NONE", "maxLLenght", std::to_string(newLineLenght).c_str(), true);
    if(iResult != 0)
        return iResult;

    iLineLenght = newLineLenght;
    return RT_OK;
}

/**
 * Lesen eines Keys in einem bestimmten Block aus der Datei als String.
 *
 * @param block: der Name des Blocks als std::string.
 * @param key: der Name des Keys als std::string.
 * @param value: ein pointer auf einen std::string, hierher wird der Wert des Keys Kopiert.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::getValue( std::string block, std::string key, std::string* value)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to get a Value from a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to get a Value from a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    // parameter prüfen
    if(value == NULL)
        return RT_ARG_INV;

    // lokale Variablen
    int iResult;
    char cLine[iLineLenght];
    char cValue[iLineLenght];

    // reset file courser to start of body
    fsetpos( pCfgFile, &fPosBody);

    // search for the block
    iResult = gotoBlock(block.c_str());
    if(iResult != 0)
    {
        if( iResult != RT_NOT_FOUND)
            ERROR("Error searching for Block in config file. Called gotoBlock() return code was " + rtToString(iResult));
        return iResult;
    }

    // search for the key
    iResult = gotoKey(key.c_str(), cLine);
    if(iResult != 0)
    {
        if( iResult != RT_NOT_FOUND)
            ERROR("Error searching for Key in config file. Called gotoKey() return code was " + rtToString(iResult));
        return iResult;
    }

    // Laden des Wertes
    iResult = valueFromKey(cLine, cValue);
    if(iResult != 0)
    {
        ERROR("Error loading Value from Key in config file. Called valueFromKey() return code was " + rtToString(iResult));
        return iResult;
    }

    *value = std::string( cValue);
    return RT_OK;
}

/**
 * Lesen eines Keys in einem bestimmten Block aus der Datei als integer.
 *
 * @param block: der Name des Blocks als std::string.
 * @param key: der Name des Keys als std::string.
 * @param value: ein pointer auf einen integer, hierher wird der Wert des Keys Kopiert.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::getValue( std::string block, std::string key, int* value)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to get a Value from a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to get a Value from a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    // lokal variablen
    int iResult;
    std::string sBuff;

    iResult = getValue( block, key, &sBuff);
    if( iResult != 0)
    {
        ERROR("Error getting Value from config file as a string, Called getValue(), return code was " + rtToString(iResult));
        return iResult;
    }

    std::stringstream ss(sBuff);
    if ( !(ss >> *value))
        return RT_WRONG_TYPE;

    return RT_OK;
}

/**
 * Lesen eines Keys in einem bestimmten Block aus der Datei als float.
 *
 * @param block: der Name des Blocks als std::string.
 * @param key: der Name des Keys als std::string.
 * @param value: ein pointer auf einen float, hierher wird der Wert des Keys Kopiert.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::getValue( std::string block, std::string key, float* value)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to get a Value from a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to get a Value from a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    // lokal variablen
    int iResult;
    std::string sBuff;

    iResult = getValue( block, key, &sBuff);
    if( iResult != 0)
    {
        ERROR("Error getting Value from config file as a string, Called getValue(), return code was " + rtToString(iResult));
        return iResult;
    }

    std::stringstream ss(sBuff);
    if ( !(ss >> *value))
        return RT_WRONG_TYPE;

    return RT_OK;
}

/**
 * Lesen eines Keys in einem bestimmten Block aus der Datei als bool.
 *
 * @param block: der Name des Blocks als std::string.
 * @param key: der Name des Keys als std::string.
 * @param value: ein pointer auf einen boolean, hierher wird der Wert des Keys Kopiert.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::getValue( std::string block, std::string key, bool* value)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to get a Value from a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to get a Value from a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    // lokal variablen
    int iResult;
    std::string sBuff;

    iResult = getValue( block, key, &sBuff);
    if( iResult != 0)
    {
        ERROR("Error getting Value from config file as a string, Called getValue(), return code was " + rtToString(iResult));
        return iResult;
    }

    std::stringstream ss(sBuff);
    if ( !(ss >> std::boolalpha >> *value))
        if( !(ss >> std::noboolalpha >> *value))
            return RT_WRONG_TYPE;

    return RT_OK;
}

/**
 * Schreiben eines Keys in einem bestimmten Block mit einem String-Wert. Wenn der Key oder der Block nicht existiert wird er neu erstellt.
 *
 * @param block: der Name des Blocks als std::string.
 * @param key: der Name des Keys als std::string.
 * @param value: Der Wert des neuen Keys als String.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::addValue( std::string block, std::string key, std::string value)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to add a Value to a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to add a Value to a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    return addValueRaw(block.c_str(), key.c_str(), std::string("\"" + value + "\"").c_str());
}

/**
 * Schreiben eines Keys in einem bestimmten Block mit einem cString-Wert. Wenn der Key oder der Block nicht existiert wird er neu erstellt.
 *
 * @param block: der Name des Blocks als std::string.
 * @param key: der Name des Keys als std::string.
 * @param value: Der Wert des neuen Keys als cString.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::addValue( std::string block, std::string key, const char* value)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to add a Value to a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to add a Value to a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    std::string s = value;

    return addValueRaw(block.c_str(), key.c_str(), std::string("\"" + s + "\"").c_str());
}

/**
 * Schreiben eines Keys in einem bestimmten Block mit einem Integer-Wert. Wenn der Key oder der Block nicht existiert wird er neu erstellt.
 *
 * @param block: der Name des Blocks als std::string.
 * @param key: der Name des Keys als std::string.
 * @param value: Der Wert des neuen Keys als Integer.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::addValue( std::string block, std::string key, int value)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to add a Value to a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to add a Value to a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    return addValueRaw(block.c_str(), key.c_str(), std::to_string(value).c_str());
}

/**
 * Schreiben eines Keys in einem bestimmten Block mit einem Float-Wert. Wenn der Key oder der Block nicht existiert wird er neu erstellt.
 *
 * @param block: der Name des Blocks als std::string.
 * @param key: der Name des Keys als std::string.
 * @param value: Der Wert des neuen Keys als float.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::addValue( std::string block, std::string key, float value)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to add a Value to a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to add a Value to a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    return addValueRaw(block.c_str(), key.c_str(), std::to_string(value).c_str());
}

/**
 * Schreiben eines Keys in einem bestimmten Block mit einem Bool-Wert. Wenn der Key oder der Block nicht existiert wird er neu erstellt.
 *
 * @param block: der Name des Blocks als std::string.
 * @param key: der Name des Keys als std::string.
 * @param value: Der Wert des neuen Keys als bool.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::addValue( std::string block, std::string key, bool value)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to add a Value to a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to add a Value to a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    std::string s = "false";
    if( value)
        s = "true";

    return addValueRaw(block.c_str(), key.c_str(), s.c_str());
}

/**
 * Löschen eines Keys in einem bestimmten Block, wenn dieser Existiert.
 *
 * @param block: der Name des Blocks als std::string.
 * @param key: der Name des Keys als std::string.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::deleteKey( std::string block, std::string key)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to delete a Key from a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to delete a Key from to a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    return addValueRaw(block.c_str(), key.c_str(), NULL);
}

/**
 * Löschen eines Blocks ( mit all seinen keys) aus der Config Datei, wenn dieser Existiert.
 *
 * @param block: der Name des Blocks als std::string.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::deleteBlock( std::string block)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to delete a Block from a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to delete a Block from a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    // lokale variablen
    char cBuff[iLineLenght];
    char cLine[iLineLenght];
    char cOrLine[iLineLenght];
    int iResult = 0;
    FILE* pTempCfg = NULL;;
    bool bCopy = true;
    std::string sTempCfgPath = pTarsLib->getPathToRoot() + "cfg/" + sName + "_new_temp" + ".cfg";

    // temp datei öffnen
    pTempCfg = fopen(sTempCfgPath.c_str(), "w");
    if(pTempCfg == NULL)
    {
        ERROR("Error opening temp config file with fopen for changeing. ERRNO=" + errno);
        return RT_FILE_ERROR;
    }

    // reset file courser to the start
    rewind(pCfgFile);

    // alles kopieren außer den block der gelöscht werden soll
    while( true)
    {

        if( fgets(cLine, iLineLenght, pCfgFile) == NULL)
        {
            if(feof(pCfgFile) != 0)
                break; // wir sind fertig

            if(ferror(pCfgFile) != 0)
            {
                fclose(pTempCfg); // fehler
                return RT_FILE_ERROR;
            }
        }

        if( bCopy == true)
            strcpy( cOrLine, cLine);

        prepareLine(cLine);
        if(cLine[0] == '[')
        {
            if( bCopy == false)
                bCopy = true; // der nächste  block ist erreicht, wieder kopieren
            else
            {
                // es ist ein Block, Namen überprüfen
                iResult = nameFromBlock( cLine, cBuff);
                if( iResult != 0)
                    WARNING("Error reading name of Block from the config file, File is probably damaged. Called nameFromBlock() return code was " + rtToString(iResult));
                if( strcmp( cBuff, block.c_str()) == 0)
                    bCopy = false; // unser zu löschender block, nicht mehr kopieren
            }
        }

        if( bCopy)
        {
            if( fputs( cOrLine, pTempCfg) < 0)
            {
                ERROR("Error writing the Line to the pTempCfg file.");
                fclose(pTempCfg);
                return RT_FILE_ERROR;
            }
        }
    }

    // die alte datei mit der neuen überschreiben
    fclose(pTempCfg);
    iResult = unload();
    if( iResult != 0)
    {
        ERROR("Error unloading config file. Called unload(), return code was: " + rtToString(iResult));
        return iResult;
    }

    if( remove( sCfgFilePath.c_str()) != 0)
    {
        ERROR("Error removeing old config file.");
        return RT_FILE_ERROR;
    }
    if( rename( sTempCfgPath.c_str(), sCfgFilePath.c_str()) != 0)
    {
        ERROR("Error renaming temp config file.");
        return RT_FILE_ERROR;
    }

    iResult = load();
    if( iResult != 0)
    {
        ERROR("Error loading config file. Called load(), return code was: " + rtToString(iResult));
        return iResult;
    }

    return RT_OK;
}

/**
 * Fügt ein Kommentar in der Zeile nach dem block oder Key ein, Key ist optional
 *
 * @param comment: das Kommentar als std::string.
 * @param block: der Block, hinter dem das Kommentar eingefügt werden soll als std::string.
 * @param key: der Key, hinter dem das Kommentar eingefügt werden soll als std::string.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::createCommentAfter( std::string comment, std::string block, std::string key)
{
   //parameter prüfen


    // lokale variablen
    char cBuff[iLineLenght];
    int iResult = 0;
    FILE* pTempCfg = NULL;;
    std::string sTempCfgPath = pTarsLib->getPathToRoot() + "cfg/" + sName + "_new_temp" + ".cfg";

    // temp datei öffnen
    pTempCfg = fopen(sTempCfgPath.c_str(), "w");
    if(pTempCfg == NULL)
    {
        ERROR("Error opening temp config file with fopen for changeing. ERRNO=" + errno);
        return RT_FILE_ERROR;
    }

    // reset file courser to the start
    rewind(pCfgFile);

    // den Block suchen
    iResult = gotoBlock( block.c_str(), pTempCfg);
    if( iResult != 0)
    {
        ERROR("Error searching for Block in config file, Called gotoBlock(), return code was " + rtToString(iResult));
        fclose(pTempCfg);
        return iResult;
    }

    if( key != "_NONE")
    {
        // den Key suchen
        iResult = gotoKey( key.c_str(), cBuff, pTempCfg);
        if( iResult != 0 && iResult != RT_NOT_FOUND)
        {
            ERROR("Error searching for Key in config file, Called gotoKey(), return code was " + rtToString(iResult));
            fclose(pTempCfg);
            return iResult;
        }

        // den key schreiben
        if( fputs( cBuff, pTempCfg)  < 0)
        {
            fclose(pTempCfg);
            ERROR("Fehler beim schreiben des Rests in die neue Cfg Datei.");
            return RT_FILE_ERROR;
        }
    }

    // das Kommentar schreiben
    iResult = fprintf( pTempCfg, "#%s\n" , comment.c_str());
    if( iResult < 0)
    {
        fclose(pTempCfg);
        ERROR("Fehler beim schreiben des Kommentars in der cfg Datei.");
        return RT_FILE_ERROR;
    }

    // den rest der Datei kopieren
    while( true)
    {
        if( fgets( cBuff, iLineLenght, pCfgFile) == NULL)
        {
            if( feof(pCfgFile) != 0)
            {
                break;
            }
            else
            {
                fclose(pTempCfg);
                ERROR("Fehler beim lesen des Rests aus der alten Cfg Datei.");
                return RT_FILE_ERROR;
            }
        }
        if( fputs( cBuff, pTempCfg)  < 0)
        {
            fclose(pTempCfg);
            ERROR("Fehler beim schreiben des Rests in die neue Cfg Datei.");
            return RT_FILE_ERROR;
        }
    }

    // die alte datei mit der neuen überschreiben
    fclose(pTempCfg);
    iResult = unload();
    if( iResult != 0)
    {
        ERROR("Error unloading config file. Called unload(), return code was: " + rtToString(iResult));
        return iResult;
    }

    if( remove( sCfgFilePath.c_str()) != 0)
    {
        ERROR("Error removeing old config file.");
        return RT_FILE_ERROR;
    }
    if( rename( sTempCfgPath.c_str(), sCfgFilePath.c_str()) != 0)
    {
        ERROR("Error renaming temp config file.");
        return RT_FILE_ERROR;
    }

    iResult = load();
    if( iResult != 0)
    {
        ERROR("Error loading config file. Called load(), return code was: " + rtToString(iResult));
        return iResult;
    }

    return RT_OK;
}

/**
 * Sucht den nächsten Block über den internen Iterator und kopiert den Namen nach blockName.
 *
 * @param blockName: zeiger auf einen std::string, hier wird der Blockname gespeichert wenn ein Block gefunden wurde.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat, RT_NOT_FOUND wenn es keinen weiteren Block gibt. Siehe libTars.h für mehr infos.
 */
int Ccfg::nextBlock( std::string* blockName)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to add a Value to a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to add a Value to a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    // parameter prüfen
    if( blockName == NULL)
        return RT_ARG_INV;

    // lokale variablen
    char cLine[iLineLenght];
    char cBuff[iLineLenght];
    int iResult = 0;

    // fPosIter setzen
    fsetpos( pCfgFile, &fPosIter);

    // den nächsten block suchen
    while( true)
    {
        if( fgets(cLine, iLineLenght, pCfgFile) == NULL)
        {
            if(feof(pCfgFile) != 0) // kein nächster
                return RT_NOT_FOUND;
            if(ferror(pCfgFile) != 0)
                return RT_FILE_ERROR;
        }

        prepareLine(cLine);
        if(cLine[0] == '[')
        {
            // es ist ein Block, Namen überprüfen
            iResult = nameFromBlock( cLine, cBuff);
            if( iResult != 0)
            {
                ERROR("Error reading name of Block from the config file, File is probably damaged. Called nameFromBlock() return code was " + rtToString(iResult));
                return iResult;
            }
            break;
        }
    }

    *blockName = cBuff;
    iterCurrBlockName = *blockName;
    fgetpos( pCfgFile, &fPosIter); // iterator weitersetzen
    return RT_OK;
}

/**
 * Sucht den nächsten Key über den internen Iterator und kopiert den Namen des Blocks, den Namen des Keys und den Wert des Keys in die entsprechenden std::strings.
 *
 * @param blockName: zeiger auf einen std::string, hier wird der Blockname gespeichert indem sich der key befindet.
 * @param keyName: zeiger auf einen std::string, hier wird der Keyname gespeichert.
 * @param keyValue: zeiger auf einen std::string, hier wird der Wert des Keys gespeichert.
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat, RT_NOT_FOUND wenn es keinen weiteren Key gibt. Siehe libTars.h für mehr infos.
 */
int Ccfg::nextKey( std::string* blockName, std::string* keyName, std::string* keyValue)
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to add a Value to a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to add a Value to a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

        // parameter prüfen
    if( blockName == NULL || keyName == NULL || keyValue == NULL)
        return RT_ARG_INV;

    // lokale variablen
    char cLine[iLineLenght];
    char cBuff[iLineLenght];
    int iResult = 0;

    // fPosIter setzen
    fsetpos( pCfgFile, &fPosIter);

    // den nächsten block suchen
    while( true)
    {
        if( fgets(cLine, iLineLenght, pCfgFile) == NULL)
        {
            if(feof(pCfgFile) != 0) // kein nächster
                return RT_NOT_FOUND;
            if(ferror(pCfgFile) != 0)
                return RT_FILE_ERROR;
        }

        prepareLine(cLine);
        if(cLine[0] == '[')
        {
            // es ist ein Block, Namen überprüfen
            iResult = nameFromBlock( cLine, cBuff);
            if( iResult != 0)
            {
                ERROR("Error reading name of Block from the config file, File is probably damaged. Called nameFromBlock() return code was " + rtToString(iResult));
                return iResult;
            }
            iterCurrBlockName = cBuff;
            continue;
        }
        else if(cLine[0] != '\0')
        {
            iResult = nameFromKey( cLine, cBuff);
            if( iResult != 0)
            {
                ERROR("Error reading name of Key from the config file, File is probably damaged. Called nameFromKey() return code was " + rtToString(iResult));
                return iResult;
            }

            *keyName = cBuff;

            iResult = valueFromKey( cLine, cBuff);
            if( iResult != 0)
            {
                ERROR("Error reading value of Key from the config file, File is probably damaged. Called valueFromKey() return code was " + rtToString(iResult));
                return iResult;
            }

            *keyValue = cBuff;
            break;
        }
    }


    *blockName = iterCurrBlockName;
    fgetpos( pCfgFile, &fPosIter); // iterator weitersetzen
    return RT_OK;
}

/**
 * Zurücksetzen des internen Iterators an den Body anfang der config Datei
 *
 * @return einen wert aus RT_* RT_OK (= 0) wenn alles geklappt hat. Siehe libTars.h für mehr infos.
 */
int Ccfg::iteratorReset()
{
    if( !isReady())
    {
        if(LIBTARS_READY)
        {
            ERROR("Trying to add a Value to a config file, but config file is not ready!");
        }
        else
            std::cerr << "Trying to add a Value to a config file, but config file is not even ready! AT: " + FILEPOS + "\n";
        return RT_NOT_LOADED;
    }

    iterCurrBlockName = "NONE";
    fPosIter = fPosBody;
    return RT_OK;
}
//-------------------------------------------------------------------
