#ifndef INCL_GTK_HELPER_IMAGE_H
#define INCL_GTK_HELPER_IMAGE_H

#include "general.h"
#include <gtk/gtk.h>

#include <string>

namespace Gtk_Helper {

class Image : Gtk_Object
{
    GtkWidget *img;

    public:
        GtkWidget* get_raw_ui_ptr() { return this->img; }
        operator GtkWidget* () { return this->img; }

        Image(const std::string& path)
            : img(gtk_image_new_from_file(path.c_str()))
        {}

    protected:
        void draw() { gtk_widget_show(img); }

        void set_from_png_buff(unsigned len, const void* buf)
        {
            auto pb_loader = gdk_pixbuf_loader_new_with_type("png", NULL);
            bool ok = gdk_pixbuf_loader_write(pb_loader, (const guchar*)buf, len, NULL);
            ok = gdk_pixbuf_loader_close(pb_loader, NULL);
            ok = ok; // TODO
            auto pb =  gdk_pixbuf_loader_get_pixbuf(pb_loader);
            gtk_image_set_from_pixbuf(GTK_IMAGE(this->img), pb);
        }

        void set_from_file(const std::string &path)
        {
            gtk_image_set_from_file(GTK_IMAGE(this->img), path.c_str());
        }
};

} /* Gtk_Helper */

#endif /* INCL_GTK_HELPER_IMAGE_H */
