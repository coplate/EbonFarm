/* DESCRIPTION
  Saves and loads character mappings from file. A character with a specific
  color translates to a nethack graphic.
*/

#ifndef NG_CHAR_SAVER
#define NG_CHAR_SAVER

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdint.h>

#include "SGRAttribute.hpp"

class CharSaver
{
    public:
        //constructor
        CharSaver(void);

        //destructor
        ~CharSaver(void);

        //Loads known char-to-sprite mappings from file.
        bool load(const std::string &pathAndName,
                  std::map <std::string, int> &knownChars);

        //Saves known char-to-sprite mappings
        bool save(const std::string &pathAndName,
                  std::map <std::string, int> &knownChars);

    private:
        //Each element has a text name for the corresponding SGR Color.
        //See SGRAttributes class.
        std::vector <std::string> colorNames;

        //Extract the information from a single line from the file
        bool processLine(std::string &oneLine,
                         std::map <std::string, int> &knownChars);

        //converts a color in text to its SGR Attribute value
        bool findColor(const std::string &theColor,
                       uint8_t &colorValue);

        //true if we encountered errors while loading, don't save the character
        //mappings
        bool loadErrors;

};//CharSaver

#endif
