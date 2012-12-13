#ifndef INCL_IMAGE_H
#define INCL_IMAGE_H

#include "gtk_helper/image.h"
#include <string>

class Image : public Gtk_Helper::Image
{
    const std::string path;

    public:
        Image(const std::string &deferred_path, const std::string &temp_path)
            : Gtk_Helper::Image(temp_path), path(deferred_path)
        {}

        virtual const std::string& get_path() const { return path; }

        template <class Img_Buffer>
        void update(Img_Buffer *img)
        {
            Global_UI_Guard ui_guard;
            this->set_from_png_buff(img->get_length(), img->get_buf());
            this->draw(); 
        }
};


class Image_From_File : public Gtk_Helper::Image
{
    public:
        Image_From_File(const std::string &temp_path)
            : Gtk_Helper::Image(temp_path)
        {}

        void update(const std::string &path)
        {
            Global_UI_Guard ui_guard;
            this->set_from_file(path);
            this->draw();
        }
};


#endif /* INCL_IMAGE_H */
