/* DESCRIPTION
  Contains the comments for a single entry in the config file.
  Used by the ConfigWriter class.
*/

#ifndef PG_CONFIG_COMMENTS
#define PG_CONFIG_COMMENTS

#include <vector>
#include <string>

class ConfigComments
{
    public:
        //constructor
        ConfigComments(void);

        //destructor
        ~ConfigComments(void);

        //add a comment for this entry
        void add(std::string newComment);

        //returns the comment at the specified index
        std::string get(int index);

        //returns the number of comments being stored
        unsigned int size(void);

    private:
        std::vector <std::string> theComments;//the comments we're storing

};//ConfigComments

#endif
