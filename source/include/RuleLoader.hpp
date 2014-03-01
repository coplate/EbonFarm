/* DESCRIPTION
  Loads rules about when to show graphics and when to show only text from file.
  Used because text menus overlap onto the game area, for example the player's inventory.
  Whenever an inventory is displayed, we want to disable graphics so the inventory shows
  up as text. Same for any other menu.
*/

#ifndef NG_RULE_LOADER
#define NG_RULE_LOADER

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "FXRule.hpp"

enum NGR_State
{
    NGRS_START, NGRS_NEW_ENTRY, NGRS_GOT_CONDITION
};//NGR_State

class RuleLoader
{
    public:
        //constructor
        RuleLoader(void);

        //destructor
        ~RuleLoader(void);

        //load a set of rules from file
        bool load(const std::string &fileName);

        //Returns true if we found a rule that matches firstLine, and sets
        //useGraphics to true or false. useGraphics is not changed if checkActions
        //returns false.
        bool checkActions(const std::string &firstLine,
                          bool &useGraphics);

    private:
        //A list of rules that we've loaded from file
        std::vector <FXRule*> theRules;

        //The current rule to add to the list
        FXRule *currentRule;

        //Current state in the FSM for accepting rules
        NGR_State myState;

        //Count the lines in the file as we process them, so if there
        //are errors we can output the line number
        int lineNumber;

        //*************** Functions ***************

        //Reset the FSM variables, should be called before opening a new file
        void resetFSM(void);

        //Handle a single line from the file. Returns false if the line was malformed
        bool processLine(std::string &oneLine);

        //We're expecting oneLine to consist of a command type, followed by
        //a parameter. Extract these and place them in theCommand and theParameter,
        //returns false if the line is malformed.
        bool getCommand(std::string &oneLine,
                        std::string &theCommand,
                        std::string &theParameter);

};//RuleLoader

#endif
