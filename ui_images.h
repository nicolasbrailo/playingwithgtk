#ifndef INCL_UI_IMAGES_H
#define INCL_UI_IMAGES_H


#include <gtk/gtk.h>


#include <string>
using std::string;
class Gtk_Image_From_File
{
    GtkWidget *img;
    public:
    operator GtkWidget* () { return this->img; }

    Gtk_Image_From_File(const string& fname)
    {
        this->img = gtk_image_new_from_file(fname.c_str());
    }
};

class Gtk_Image_From_PNG_Buff
{
    GtkWidget *img;
    public:
    operator GtkWidget* () { return this->img; }

    Gtk_Image_From_PNG_Buff(unsigned len, const void* buf)
    {
        auto pb_loader = gdk_pixbuf_loader_new_with_type("png", NULL);
        bool ok = gdk_pixbuf_loader_write(pb_loader, (const guchar*)buf, len, NULL);
        ok = gdk_pixbuf_loader_close(pb_loader, NULL);
        ok = ok;
        auto pb =  gdk_pixbuf_loader_get_pixbuf(pb_loader);
        this->img = gtk_image_new_from_pixbuf(pb);
    }
};


class Gtk_Image_From_Pixbuf_Buffer
{
    GtkWidget *img;
    public:
    operator GtkWidget* () { return this->img; }

    Gtk_Image_From_Pixbuf_Buffer(unsigned len, const void* buf)
    {
        len = len;
        auto pb = gdk_pixbuf_new_from_data((const guchar*)buf,
                            GDK_COLORSPACE_RGB, false,
                            8, 150, 150, 452,
                            NULL, NULL);
        this->img = gtk_image_new_from_pixbuf(pb);
    }
};

#endif /* UI_IMAGES_H */
