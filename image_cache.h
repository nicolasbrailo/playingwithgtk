#ifndef INCL_IMAGES_CACHE_H
#define INCL_IMAGES_CACHE_H

// Used to get the UI_Images IMPL for each cache type
#include "ui_images.h"

#include <string>
#include <map>
using std::string;
using std::map;


// TODO: Configure resize size

class Image_Cache
{
    public:
    /**
     * A Mem_Image represents the cached data of an image in whatever
     * format the subclass defines (this is determined by the load_image
     * method).
     **/
    class Mem_Image
    {
        unsigned length;
        char *buf;

        public:
        unsigned get_length() const { return length; }
        const char* get_buf() const { return buf; }
        Mem_Image(unsigned length, const void *buf);
        ~Mem_Image();
    };

    Image_Cache();
    ~Image_Cache();

    /**
     * Gets an image from the cache; if it's not already cached then it's the
     * subclass' responsibility to load it from img_path.
     * @param string img_path  The path to the image (not necesarily filesystem)
     * @returns A cached image object
     */
    const Mem_Image* operator[] (const string &img_path);

    protected:
    /**
     * Subclass responsibility. Loads an image not present in the cache.
     * eg. load_image will get an image from the filesystem and return a
     * Mem_Image object containing a buffer with the image in PNG format.
     **/
    virtual Mem_Image* load_image(const string &img_path) const = 0;

    private:
    map<const string, Mem_Image*> cache;
};


/**
 * Gets an image from the file system and resizes it using a GDK pixbuf object.
 * Will store the image directly in pixbuf format.
 */
class Pixbuf_Resize_Thumbnail_Cache : public Image_Cache
{
    protected:
    Image_Cache::Mem_Image* load_image(const string &path) const;

    public:
    typedef Gtk_Image_From_Pixbuf_Buffer UI_Image_Impl;
};


/**
 * Gets an image from the filesystem and resizes it using ImageMagick.
 * Can read pretty much any type of images. Will cache the image in PNG format.
 */
class Magick_Thumbnail_Cache : public Image_Cache
{
    const string &width;

    public:
    Magick_Thumbnail_Cache(const string &width) : width(width) {}
    typedef Gtk_Image_From_PNG_Buff UI_Image_Impl;

    protected:
    Image_Cache::Mem_Image* load_image(const string &path) const;
};


#endif /* INCL_IMAGES_CACHE_H */
