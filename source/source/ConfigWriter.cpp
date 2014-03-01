#include "ConfigWriter.hpp"
#include <math.h>

using namespace std;

string ConfigWriter::path;

ConfigWriter::ConfigWriter(void)
{
}//constructor

ConfigWriter::~ConfigWriter(void)
{
}//destructor

void ConfigWriter::setPath(const std::string &newPath)
{
    path = newPath;
}//setPath

string ConfigWriter::loadString(const string &fileName,
                                const string &configID)

{
    ifstream infile;//obtains input from the file
    string result;//the loaded string to return
    string oneLine;//a single line of text from the file
    string pathAndName;//the path to the file, including the file name
    bool foundID = false;//true if we found the configID

    //### Open the file ###
    pathAndName = path;
    pathAndName.append(fileName);
    infile.open(pathAndName.c_str());
    if (!infile.good())
    {
        cout << "ConfigWriter couldn't open " << pathAndName << " for reading" << endl;
        throw 1;
    }//if !infile

    //### Find the configID ###
    while ((infile.good()) && (!foundID))
    {
        //read the configID line
        getline(infile, oneLine);
        oneLine = cleanLine(oneLine);

        if (infile.good())
        {
            if (oneLine.size() > 0)
            {
                if (oneLine.at(0) == '%')
                {
                    //remove the % symbol
                    oneLine.erase(0, 1);

                    if (oneLine.size() > 0)
                    {
                        //check for matches
                        if (oneLine == configID)
                            foundID = true;
                    }//if size()
                    else
                    {
                        cout << "File " << pathAndName << " contains a % with no identifier" << endl;
                        throw 1;
                    }//else size
                }//if oneLine
            }//if size()
        }//if good()
    }//while good() && !foundID

    //### See if the ID was found ###
    if (!foundID)
    {
        cout << "ConfigWriter couldn't find " << configID << " in file " << pathAndName << endl;
        throw 1;
    }//if !foundID

    //### Read the value ###
    getline(infile, result);
    result = cleanLine(result);
    if (!infile.good())
    {
        if (!infile.eof())
        {
            cout << "ConfigWriter couldn't read file after " << configID << " in file " << pathAndName << endl;
            throw 1;
        }//if !eof()
    }//if !good()

    //### Verify that we read something ###
    if (result.size() <= 1)
    {
        cout << "ConfigWriter found no value for " << configID << " in file " << pathAndName << endl;
        throw 1;
    }//if size()

    //### Remove the > symbol ###
    if (result.at(0) == '>')
        result.erase(0, 1);
    else
    {
        cout << "ConfigWriter expected > to indicate the value for " << configID
             << " in file " << pathAndName << endl;
        throw 1;
    }//if result

    //### Close the file ###
    if (infile.is_open())
        infile.close();

    return result;
}//loadString

string ConfigWriter::cleanLine(const string &oneLine)
{
    string result;//the cleaned line to return
    char currentChar = ' ';//the current character to examine

    for (unsigned int i = 0; i < oneLine.size(); i++)
    {
        currentChar = oneLine.at(i);

        if ((currentChar >= 32) && (currentChar <= 126))
            result.push_back(currentChar);
    }//for i

    return result;
}//cleanLine

void ConfigWriter::writeString(const string &fileName,
                               const string &configID,
                               const string &newString)
{
    vector <ConfigComments*> fileComments;//the comments for all entries
    vector <string> fileIDs;//every configID read from the file
    vector <string> fileValues;//every string value from the file
    ConfigComments *commentGroup = NULL;//a group of comments for a single entry
    ifstream infile;//obtains input from the file
    ofstream outfile;//re-writes the file
    string pathAndName;////the path to the file, including the file name
    string oneLine;//a single line of text from the file
    bool foundValue = false;//true if configID was present in the file

    //### Read the entire file into memory ###
    pathAndName = path;
    pathAndName.append(fileName);
    infile.open(pathAndName.c_str());
    if (infile.good())
    {
        while (infile.good())
        {
            getline(infile, oneLine);
            oneLine = cleanLine(oneLine);

            if (infile.good())
            {
                if (oneLine.size() > 0)
                {
                    if (readEntry(oneLine, infile, configID, newString,
                        fileComments, fileIDs, fileValues, pathAndName))
                    foundValue = true;
                }//if size()
            }//if good()
        }//while good()
    }//if good()

    //### Close the input file ###
    if (infile.is_open())
        infile.close();

    //### Open the output file ###
    outfile.open(pathAndName.c_str());
    if (!outfile.good())
    {
        cout << "ConfigWriter couldn't open " << pathAndName << " for writing" << endl;
        throw 1;
    }//if !good()

    //### Write the new config file ###
    for (unsigned int i = 0; i < fileComments.size(); i++)
    {
        commentGroup = fileComments.at(i);
        for (unsigned int j = 0; j < commentGroup->size(); j++)
            outfile << commentGroup->get(j) << endl;

        outfile << fileIDs.at(i) << endl;
        outfile << fileValues.at(i) << endl;
        outfile << endl;

        if (!outfile.good())
        {
            cout << "ConfigWriter: error writing to file " << pathAndName << endl;
            throw 1;
        }//if !good()
    }//for i

    //### Create a new entry if no existing entry was found ###
    if (!foundValue)
    {
        outfile << "%" << configID << endl;
        outfile << ">" << newString << endl;
        outfile << endl;
        if (!outfile.good())
        {
            cout << "ConfigWriter: error writing to file " << pathAndName << endl;
            throw 1;
        }//if !good()
    }//if !foundValue

    //### Close the output file ###
    if (outfile.is_open())
        outfile.close();

    //### Free memory ###
    for (unsigned int i = 0; i < fileComments.size(); i++)
    {
        delete fileComments.at(i);
        fileComments.at(i) = NULL;
    }//for i
    fileComments.clear();
}//writeString

bool ConfigWriter::readEntry(string oneLine,
                             ifstream &infile,
                             const string &configID,
                             const string &newString,
                             vector <ConfigComments*> &fileComments,
                             vector <string> &fileIDs,
                             vector <string> &fileValues,
                             const string &pathAndName)
{
    ConfigComments *commentGroup = NULL;//a group of comments for a single entry
    string currentID;//the current config ID read from file
    string newValue;//the new value to use, equal to > plus newString
    bool result = false;//true if we found configID and replaced it's value with newString
    bool readComments = true;//set to false when we've finished reading comments

    //### Verify that oneLine is a comment or ID ###
    if ((oneLine.at(0) != '#') && (oneLine.at(0) != '%'))
    {
        cout << "ConfigWriter error reading " << pathAndName << ": " << oneLine << endl;
        cout << "Expected a comment or ID" << endl;
        throw 1;
    }//if oneLine && oneLine

    //### Read comments ###
    commentGroup = new ConfigComments;
    fileComments.push_back(commentGroup);
    if (oneLine.at(0) == '#')
    {
        commentGroup->add(oneLine);

        while (readComments)
        {
            getline(infile, oneLine);
            oneLine = cleanLine(oneLine);

            if (!infile.good())
            {
                cout << "ConfigWriter couldn't read from file " << pathAndName << endl;
                cout << "Expected config ID after comments" << endl;
                throw 1;
            }//if !good()

            if (oneLine.size() == 0)
            {
                cout << "ConfigWriter error reading from file " << pathAndName << endl;
                cout << "Got a blank line; expected comment or ID" << endl;
                throw 1;
            }//if size()

            if (oneLine.at(0) == '#')
                commentGroup->add(oneLine);
            else if (oneLine.at(0) == '%')
                readComments = false;
            else
            {
                cout << "ConfigWriter error reading from file " << pathAndName << endl;
                cout << "Invalid line after comments section" << endl;
                throw 1;
            }//else oneLine
        }//while readComments
    }//if oneLine

    //### Verify that there's a config ID ###
    if (oneLine.size() <= 1)
    {
        cout << "ConfigWriter: expected config ID after % symbol" << " in file " << pathAndName << endl;
        throw 1;
    }//if size()

    //### See if the config ID matches the ID to change ###
    fileIDs.push_back(oneLine);
    currentID = oneLine;
    currentID.erase(0, 1);
    if (currentID == configID)
    {
        //cout << "Found ID " << configID << endl;
        result = true;
    }//if currentID

    //### Read the value ###
    getline(infile, oneLine);
    oneLine = cleanLine(oneLine);
    if (!infile.good())
    {
        if (!infile.eof())
        {
            cout << "ConfigWriter: expected value after ID " << fileIDs.at(fileIDs.size() - 1)
                 << " in file " << pathAndName << endl;
            throw 1;
        }//if !eof()
    }//if !good()

    //### Verify that there was a value present in the file ###
    if (oneLine.size() <= 1)
    {
        cout << "ConfigWriter: expected value after ID " << fileIDs.at(fileIDs.size() - 1)
             << " in file " << pathAndName << endl;
        throw 1;
    }//if size()

    //### Verify that the value is preceeded by > ###
    if (oneLine.at(0) != '>')
    {
        cout << "ConfigWriter expected > to preceed the value for "
             << fileIDs.at(fileIDs.size() - 1) << " in file " << pathAndName << endl;
        throw 1;
    }//if oneLine

    //### Use the new value, if the config ID matches ###
    if (result)
    {
        newValue = ">";
        newValue.append(newString);
        fileValues.push_back(newValue);
    }//if result
    else
        fileValues.push_back(oneLine);

    return result;
}//readEntry

int ConfigWriter::loadInt(const string &fileName,
                          const string &configID)
{
    string stringValue;//the string read from file
    string pathAndName;//the path to the file, including the file name
    int result = 0;//the integer to return

    pathAndName = path;
    pathAndName.append(fileName);

    stringValue = loadString(fileName, configID);
    if (!strToInt(stringValue, result))
    {
        cout << "For file " << pathAndName << ", label " << configID << ", expected int" << endl;
        throw 1;
    }//if strToInt()

    return result;
}//loadInt

void ConfigWriter::writeInt(const string &fileName,
                            const string &configID,
                            int newInt)
{
    writeString(fileName, configID, intToStr(newInt));
}//writeInt

bool ConfigWriter::loadBool(const string &fileName,
                            const string &configID)
{
    string stringValue;//the string read from file
    string pathAndName;////the path to the file, including the file name
    bool result = true;//the boolean to return

    pathAndName = path;
    pathAndName.append(fileName);

    stringValue = loadString(fileName, configID);
    if (stringValue == "TRUE")
        result = true;
    else if (stringValue == "FALSE")
        result = false;
    else
    {
        cout << "For file " << pathAndName << ", label " << configID << ", expected TRUE or FALSE" << endl;
        throw 1;
    }//if strToInt()

    return result;
}//loadBool

void ConfigWriter::writeBool(const string &fileName,
                             const string &configID,
                             bool newBool)
{
    if (newBool)
        writeString(fileName, configID, "TRUE");
    else
        writeString(fileName, configID, "FALSE");
}//writeBool

double ConfigWriter::loadDouble(const std::string &fileName,
                                const std::string &configID)
{
    stringstream converter;//converts the string to a double
    string stringValue;//the string read from file
    string pathAndName;////the path to the file, including the file name
    double result = 0;//the value to return

    pathAndName = path;
    pathAndName.append(fileName);
    stringValue = loadString(fileName, configID);

    converter << stringValue;
    converter >> result;
    if ((converter.fail()) || (!converter.eof()))
    {
        if (stringValue == "nan")
            result = NAN;
        else if (stringValue == "inf")
            result = INFINITY;
        else
        {
            cout << "For file " << pathAndName << ", label " << configID << ", expected a double" << endl;
            throw 1;
        }//else stringValue
    }//if fail() || !eof()

    return result;
}//loadDouble

void ConfigWriter::writeDouble(const std::string &fileName,
                               const std::string &configID,
                               double newDouble)
{
    stringstream converter;//converts newDouble to a string
    string text;//text version of newDouble

    converter << newDouble;
    converter >> text;

    writeString(fileName, configID, text);
}//writeDouble

bool ConfigWriter::strToInt(const string &text,
                            int &theInt)
{
    stringstream converter;//converts text to an int
    int tempValue = 0;//dont change the parameter if there were errors
    bool result = true;//false on conversion error

    //### Verify there's a token in text ###
    if (converter.rdstate() & stringstream::eofbit)
        result = false;

    //### Do the conversion ###
    if (result)
    {
        converter << text;
        converter >> tempValue;

        //couldn't convert to int
        if (converter.rdstate() & stringstream::failbit)
            result = false;

        //unknown error
        else if (converter.rdstate() & stringstream::badbit)
            result = false;

        //got an int, but there's more tokens in the text
        else if (!(converter.rdstate() & stringstream::eofbit))
            result = false;
    }//if result

    //### Use this value ###
    if (result)
        theInt = tempValue;

  return result;
}//strToInt

string ConfigWriter::intToStr(int number)
{
    string result;//string containing converted number
    stringstream converter;//does the conversion

    converter << number;
    converter >> result;

    return result;
}//intToStr
