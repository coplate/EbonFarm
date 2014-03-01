/* DESCRIPTION

  Loads a list of sprites and their descriptive names from file.
  Intended to be used with the .txt files from qt-nethack.

  imageList is a vector that stores the images.
  nameMap is an std::map where the key is an image name, and the data is an index in imageList.
  This allows us to quickly find any image given its name.
*/

#ifndef NG_IMAGE_LOADER
#define NG_IMAGE_LOADER

#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <QImage>
#include <QPixmap>

#include "NGSettings.hpp"

class WhiteBoard;

class ImageLoader
{
    public:
        //constructor
        ImageLoader(WhiteBoard *newWhiteBoard);

        //destructor
        ~ImageLoader(void);

        //Load all sprites from file
        bool loadImages(void);

        //Loads a custom tileset from the specified file, returns false on failure
        //and displays an error message
        bool loadCustom(QString filename);

        //Return the graphic at the given index in spriteList
        QPixmap* getImage(unsigned int index);

        //Returns the number of images being stored
        unsigned int numImages(void);

        //Given a sprite name, return its index in imageList, or NG_UNKNOWN
        //if not found
        int findImage(const std::string &spriteName);

        //The width and height of the default X11 tileset
        static const unsigned int DEFAULT_SPRITE_SIZE = 16;

        //The width and height of a custom tileset in tiles
        static const int CUSTOM_HORIZ_TILES = 40;
        static const int CUSTOM_VERT_TILES = 30;

    private:
        //Each element is a nethack graphic
        std::vector <QPixmap*> imageList;

        //Pointer to the global whiteboard, don't delete
        WhiteBoard *whiteBoard;

        //Map tile names to their location in the sprite list
        std::map <std::string, unsigned int> nameMap;

        //Map characters to 32-bit colors. These are found at the beginning of the
        //sprite file.
        std::map<char, QRgb> fileColors;

        //############### Functions ###############

        //true if we've loaded all the sprite data
        bool dataLoaded;

        //load graphics from the specified file
        bool loadFile(const std::string &fileName);

        //load the color scheme from the start of the file
        bool loadColors(std::ifstream &infile);

        //Add a single line of color information from the file to the map
        bool insertColor(std::string &oneLine);

        //Load a single tile from the file
        bool loadTile(std::ifstream &infile,
                      std::string &header);

        //Draw a line of graphics from the file onto oneSprite
        bool loadGraphicsLine(std::string &oneLine,
                              QImage *oneImage,
                              int yPos);

        //Store the image in the list, and the tile name in nameMap.
        //Checks for dual names, like crude dagger / orcish dagger,
        //and creates two entries if required.
        bool storeImage(std::string &tileName,
                        QImage* oneImage);

};//SpriteLoader

#endif
