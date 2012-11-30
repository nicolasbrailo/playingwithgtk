#ifndef INCL_IMAGES_CACHE_H
#define INCL_IMAGES_CACHE_H

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

    private:
    map<const string, Mem_Image*> cache;
};


#endif /* INCL_IMAGES_CACHE_H */
