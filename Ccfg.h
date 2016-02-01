/*
 * libTars
 * Ccfg.h
 *
 * @author: Hendrik Schwanekamp
 *
 * Implementiert die Klasse Ccfg, die funktionen zum verwalten einer config datei bereit stellt.
 *
 */

#ifndef CCFG_H
#define CCFG_H

// Includes
//--------------------
#include "CIfcTarsBase.h"
#include <cstdio>
#include <string>
//--------------------

/**
 * Klasse zum Verwalten einer einer Konfigurationsdatei
 * Wichtig: der Key "_NONE" ist reserviert
 */
class Ccfg : public CIfcTarsBase
{
private:
    FILE* pCfgFile;             // Pointer zur config datei
    fpos_t fPosBody;            // Dateiposition am Body-anfang

    std::string sCfgFilePath;   // Pfad zur config Datei
    std::string sName;          // Name der Config Datei (ohne endung und pfad)

    fpos_t fPosIter;                // Dateiposition des Iterators
    std::string iterCurrBlockName;  // Name des Blocks, in dem sich der Iterator momentan befindet

    int iLineLenght;            // vermutlich Maximale Zeilenlänge in der Datei
    float fVersion;             // Version der cfg datei
    std::string sDescription;   // Beschreibung der config Datei

    // hilfsfunktionen zur bearbeitung und auswertung der Strings
    int prepareLine(char* line);                            // Hilfsfunktion zum vorbereiten einer gelesenen Zeile
    int nameFromBlock(const char* line, char* blockName);   // Hilfsfunktion zum lesen eines Block-Namens
    int nameFromKey(const char *line, char* keyName);       // Hilfsfunktion zum lesen des Namens eines Schlüssels
    int valueFromKey(const char* line, char* keyValue);     // Hilfsfunktion zum lesen des Wertes eines Schlüssels

    // funktionen zum suchen von Blocks und Keys
    int gotoBlock(const char* blockName, FILE* pCopy=NULL);            // sucht einen Block in der datei und setzt den Dateicourser dort hin
    int gotoKey(const char* keyName, char* lineOut, FILE* pCopy=NULL); // sucht einen key im Aktuellen Block, wenn gefunden wird die zeile in line out gespeichert

    // funktionen zum ändern der cfg Datei mit einer temp Datei
    int addValueRaw( const char* block, const char* key, const char* value, bool header=false); // hinzufügen oder ändern eines wertes in raw

    // private updatefunktionen für header-werte
    int updateLineLenght(int newLineLenght); // updaten der vermutlich Maximale Zeilenlänge

public:
    Ccfg();
    ~Ccfg();

    // init und exit funktionen
    int init();
    int init(std::string sName);    // initialisieren der Klasse
    int exit();                     // deinitialisieren der Klasse
    int load();                     // laden aller wichtigen resourcen (sollte von init aufgerufen werden)
    int unload();                   // entladen alles wichtigen resourcen (sollte von exit aufgerufen werden)
    int reinit();
    int reinit(std::string sName);  // neuinitialisieren der Klasse

    int createNew();                                                            // neues config file mit standardwerten erstellen, zb wenn load mit RT_NO_FILE fehlschlägt
    int createNew(std::string sName, std::string sDescription, float fVersion); // neues config file mit angegebenen werten erstellen, zb wenn load mit RT_NO_FILE fehlschlägt

    // getter u setter für headerwerte
    int getLineLenght();                            // getter für die vermutlich Maximale Zeilenlänge in der Datei
    float getVersion();                             // getter für die Dateiversion
    std::string getDescription();                   // getter für die beschreibung
    int updateDescription(std::string description); // updaten der Beschreibung
    int updateVersion(float newVersion);            // updaten der Version

    // Funktionen zum lesen von Werten aus der Datei
    int getValue( std::string block, std::string key, std::string* value);  // gibt den wert von key im typ string im block block zurück
    int getValue( std::string block, std::string key, int* value);          // gibt den wert von key im typ int im block block zurück
    int getValue( std::string block, std::string key, float* value);        // gibt den wert von key im typ float im block block zurück
    int getValue( std::string block, std::string key, bool* value);         // gibt den wert von key im typ bool im block block zurück

    // Funktionen zum schreiben von Werten in die Datei
    int addValue( std::string block, std::string key, std::string value);     // fügt key im block block mit wert value hinzu, wenn forhanden überschreiben
    int addValue( std::string block, std::string key, const char* value);           // fügt key im block block mit wert value hinzu, wenn forhanden überschreiben
    int addValue( std::string block, std::string key, int value);                   // fügt key im block block mit wert value hinzu, wenn forhanden überschreiben
    int addValue( std::string block, std::string key, float value);                 // fügt key im block block mit wert value hinzu, wenn forhanden überschreiben
    int addValue( std::string block, std::string key, bool value);                  // fügt key im block block mit wert value hinzu, wenn forhanden überschreiben

    // funktionen zum löschen
    int deleteKey( std::string block, std::string key);         // löschen eines Keys aus der Datei
    int deleteBlock( std::string block);                              // löschen eines Blocks aus der Datei

    // creating and deleting comments
    int createCommentAfter( std::string comment, std::string block, std::string key ="_NONE");   // ein Kommentar in die Zeile danach schreiben

    // iterating over cfg files
    int nextBlock( std::string* blockName);                                              // den nächsten block lesen
    int nextKey( std::string* blockName, std::string* keyName, std::string* keyValue);   // den nächsten key lesen
    int iteratorReset();                                                                 // den iterator an den dateianfang zurücksetzen
};

#endif // CCFG_H
