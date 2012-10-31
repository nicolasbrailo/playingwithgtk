#ifndef INCL_GTK_HELPER_IMAGE_GRID_H
#define INCL_GTK_HELPER_IMAGE_GRID_H

#include <gtk/gtk.h>
#include "general.h"

namespace Gtk_Helper {

class Image_Grid : Gtk_Object
{
    GtkWidget *widget;

    public:
        Image_Grid()
                : widget(gtk_layout_new(NULL, NULL))
        {}

        operator GtkWidget* (){ return this->widget; }


    protected:
        unsigned get_width() const
        {
            int width, height;
            gtk_widget_get_size_request(this->widget, &width, &height);
            return width;
        }

        template <class T>
        void place_image(T *img, unsigned img_width, unsigned img_height,
                         unsigned horiz_pos_px, unsigned vert_pos_px)
        {
            gtk_widget_set_usize(img, img_width, img_height);
            gtk_layout_put(GTK_LAYOUT(this->widget), img, horiz_pos_px, vert_pos_px);
            gtk_widget_show(img);
        }

        template <class T>
        void remove_image(T *img)
        {
            gtk_widget_destroy(GTK_WIDGET(img));
        }
};

} /* namespace Gtk_Helper */

#endif /* INCL_GTK_HELPER_IMAGE_GRID_H */
