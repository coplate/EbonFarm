/* DESCRIPTION
    A single rule loaded from file. These tell EbonHack when to turn
    the graphics mode on or off, based on the contents of the first line
    from the telnet window. For example, when the first line starts with
    Amulets, the user is looking at her inventory, and graphics mode should
    be turned off.
*/

#ifndef NG_FX_RULE
#define NG_FX_RULE

#include <string>
#include <iostream>
#include <QRegExp>

enum NGFX_ActionType
{
    NGFX_ENABLE, NGFX_DISABLE
};//NGFX_ActionType

class FXRule
{
    public:
        //constructor
        FXRule(void);

        //destructor
        ~FXRule(void);

        //Store the regular expression, returns false on errors.
        //Overwrites any previously stored expression.
        bool storeCondition(std::string &prefix,
                            std::string &regExp);

        //Store the action for this rule, returns false on errors.
        bool storeAction(std::string &parameter);

        //Returns true if the conditions for this rule match the line, and
        //sets useGraphics appropriately.
        bool checkRule(const std::string &firstLine,
                       bool &useGraphics);

    private:
        //A regular expression which must be satisfied for this rule to be
        //applied.
        QRegExp theRule;

        //The action to perform when the conditions are met
        NGFX_ActionType action;

};//FXRule

#endif
