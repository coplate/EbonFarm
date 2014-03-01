#include "ConfigComments.hpp"

using namespace std;

ConfigComments::ConfigComments(void)
{
}//constructor

ConfigComments::~ConfigComments(void)
{
}//destructor

void ConfigComments::add(std::string newComment)
{
    theComments.push_back(newComment);
}//add

string ConfigComments::get(int index)
{
    return theComments.at(index);
}//get

unsigned int ConfigComments::size(void)
{
    return theComments.size();
}//size
