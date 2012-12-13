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

        const std::string& get_path() const { return path; }

        template <class Img_Buffer>
        void update(Img_Buffer *img)
        {
            Global_UI_Guard ui_guard;
            this->set_from_png_buff(img->get_length(), img->get_buf());
            this->draw(); 
        }
};

class Image_From_File : public Image
{
    public:
        // TODO: Check if upgrading gcc we can get inherit ctrs
        Image_From_File(const std::string &deferred_path, const std::string &temp_path)
                : Image(deferred_path, temp_path)
        {}

        void update(const std::string &path)
        {
            cout << "HOLA" << endl;
            Global_UI_Guard ui_guard;
            this->set_from_file(path);
            this->draw();
        }
};

#endif /* INCL_IMAGE_H */
