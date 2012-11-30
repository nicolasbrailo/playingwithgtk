#include "sync_queue.h"
#include <vector>
#include <thread>

template <class Cache, class UI_Image> class Deferred_Image_Loader
{
    Cache &cache;
    Sync_Queue<UI_Image*> queue;
    vector<thread*> executors_lst;

    public:
    Deferred_Image_Loader(Cache &cache, unsigned executors)
            : cache(cache)
    {
        for (unsigned i=0; i<executors; ++i)
        {
            auto th = new thread(&Deferred_Image_Loader<Cache, UI_Image>::executor, this);
            executors_lst.push_back(th);
        }
    }

    virtual ~Deferred_Image_Loader()
    {
        for (auto ex : executors_lst)
        {
            // TODO: Signal end
            ex->join();
        }
    }

    void process(UI_Image *img)
    {
        queue.push(img);
    }

    private:
    void executor()
    {
        while (true) {
            UI_Image* img = queue.pop();

            // Get the img path and retrieve it from cache
            const string &path = img->get_path();
            auto cached_img = cache[path];

            // Now load the image in the UI, done separatedly because it might
            // be non thread safe: this way we can paralelize the cache update
            // and serialize the ui showing, if it's needed; this way we give
            // img the chance to have a "UI mutex"
            img->update(cached_img->get_length(), cached_img->get_buf());
        }
    }
};
