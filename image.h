#ifndef INCL_IMAGE_H
#define INCL_IMAGE_H

#include "gtk_helper/image.h"
#include <string>

class Image : public Gtk_Helper::Image
{
    const std::string path;
    // TODO: Add updated stuff from class below so we dont
    // try to delete this obj if theres a thread pending to upd it

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


#include <mutex>
using std::mutex;
class Image_From_File : public Gtk_Helper::Image
{
    mutex doing_stuff;
    bool updated, delete_requested;

    public:
        Image_From_File(const std::string &temp_path)
            : Gtk_Helper::Image(temp_path), updated(false), delete_requested(false)
        {}

        ~Image_From_File()
        {
            if (not updated)
                cout << "Someone is trying to delete me but there's i'm being updated by another thread!" << endl;

            while (not updated)
                /* lock deleting thread till done */;
        }

        void be_gone()
        {
            doing_stuff.lock();
            cout << "Rq delete" << endl;
            if (updated) {
                cout << "no need to wait" << endl;
                delete this;
                return;
            }

            delete_requested = true;
            cout << "delete scheduled" << endl;
            doing_stuff.unlock();
        }

        void update(const std::string &path)
        {
            doing_stuff.lock();
            updated = true;
            if (delete_requested) {
                cout << "Update not needed me fui" << endl;
                delete this;
                return;
            }

            cout << "Update locking ui mutex" << endl;

            Global_UI_Guard ui_guard;
            this->set_from_file(path);
            this->draw();

            cout << "Unlocked" << endl;
            doing_stuff.unlock();
        }
};


#endif /* INCL_IMAGE_H */
