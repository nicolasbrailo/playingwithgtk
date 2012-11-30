#include "image_cache.h"
#include <string.h>
#include <mutex>

using namespace std;

static mutex global_cache_lock;

Image_Cache::Mem_Image* load_image(const string&);


Image_Cache::Mem_Image::Mem_Image(unsigned length, const void *buf)
        : length(length), buf(new char[length])
{
    memcpy(this->buf, buf, length);
}

Image_Cache::Mem_Image::~Mem_Image()
{
    delete[] this->buf;
}


/**********************************************************/
Image_Cache::Image_Cache() {}

Image_Cache::~Image_Cache() 
{
    for (auto i = this->cache.begin(); i != this->cache.end(); ++i)
    {
        Mem_Image *p = i->second;
        p = p;
        delete p;
    }
}

const Image_Cache::Mem_Image* Image_Cache::operator[] (const string &img_path)
{
    // try to get the img from cache
    {
        // TODO: This is read only, do we really need a lock?
        lock_guard<mutex> l(global_cache_lock);
        if (cache[img_path] != NULL) return cache[img_path];
    }

    auto img = load_image(img_path);
    {
        lock_guard<mutex> l(global_cache_lock);
        cache[img_path] = img;
        return cache[img_path];
    }
}



#include <Magick++.h>
Image_Cache::Mem_Image* load_image(const string& path)
{
    Magick::Image img;
    img.read(path);
    img.magick("png");
    img.resize("150");

    // Debug imgmagick
    //img.display();

    Magick::Blob blob;
    img.write(&blob);

    return new Image_Cache::Mem_Image(blob.length(), blob.data());
}

