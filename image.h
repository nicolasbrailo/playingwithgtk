#ifndef INCL_IMAGE_H
#define INCL_IMAGE_H

#include "gtk_helper/image.h"
#include <string>

class Image : public Gtk_Helper::Image
{
    const std::string path;
    thread *t;

    public:
        Image(const std::string &deferred_path, const std::string &temp_path)
            : Gtk_Helper::Image(temp_path), path(deferred_path)
        {}

        const std::string& get_path() const { return path; }

        template <class T> void update(T &cache)
        {
            auto mem_buf = cache[path];
            Global_UI_Guard ui_guard;
            this->set_from_png_buff(mem_buf->get_length(), mem_buf->get_buf());
            this->draw(); 
        }
};

#endif /* INCL_IMAGE_H */
