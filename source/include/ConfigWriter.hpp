/* DESCRIPTION
  A simple configuration file manager, intended to make it easy to hand edit the
  file, or to have the program update configuration values.

  File Format:
    -Each entry in the file has 0 or more comments, an identifier, and a value
    -Comments start with a #, IDs start with a %, and the value starts with a >
    -There should be no spaces between lines in a given entry

  Example:

  #This is my descriptive comment
  #Comments work great
  %My Favorite Color
  >Blue

  When you write to the file, ConfigWriter searches for the given ID, and modifies the value.
  If your ID wasn't found in the file, it creates a new entry.
  Any comments should be added by hand-editing the file.

  Note that writing a new value involves reading the whole file, changing or adding the entry,
  and re-writing the whole file. ConfigWriter is not intended to work on huge files for reasons
  of efficiency.
*/

#ifndef PG_CONFIGWRITER
#define PG_CONFIGWRITER

#include <fstream>
#include <vector>
#include <sstream>
#include <iostream>

#include "ConfigComments.hpp"

class ConfigWriter
{
    public:
        //constructor
        ConfigWriter(void);

        //destructor
        ~ConfigWriter(void);

        //Set the path to the directory containing the config file(s), newPath should end
        //with a /
        static void setPath(const std::string &newPath);

        //loads a string from the file, from the entry with the specified configID
        static std::string loadString(const std::string &fileName,
                                      const std::string &configID);

        //writes newString into the file
        static void writeString(const std::string &fileName,
                                const std::string &configID,
                                const std::string &newString);

        //loads an integer from the file
        static int loadInt(const std::string &fileName,
                           const std::string &configID);

        //writes theInt to the file
        static void writeInt(const std::string &fileName,
                             const std::string &configID,
                             int newInt);

        //Loads a boolean from the file
        static bool loadBool(const std::string &fileName,
                             const std::string &configID);

        //writes newBool to the file
        static void writeBool(const std::string &fileName,
                              const std::string &configID,
                              bool newBool);

        //Loads a double from the file
        static double loadDouble(const std::string &fileName,
                                 const std::string &configID);

        //Writes newDouble to the file
        static void writeDouble(const std::string &fileName,
                                const std::string &configID,
                                double newDouble);

        //returns the string equivalent of number
        static std::string intToStr(int number);

        //returns false if text is not a valid integer
        static bool strToInt(const std::string &text,
                             int &theInt);

    private:
        //the path to the config file, should end in /
        static std::string path;

        //Returns a copy of oneLine, with only non-displayable and non-typeable characters removed
        static std::string cleanLine(const std::string &oneLine);

        //Read a single entry from the file, and store it in the provided
        //variables. Returns true if configID was found, and changes the value
        //to newString
        static bool readEntry(std::string oneLine,
                              std::ifstream &infile,
                              const std::string &configID,
                              const std::string &newString,
                              std::vector <ConfigComments*> &fileComments,
                              std::vector <std::string> &fileIDs,
                              std::vector <std::string> &fileValues,
                              const std::string &pathAndName);

};//ConfigWriter

#endif
