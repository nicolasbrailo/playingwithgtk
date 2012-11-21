#ifndef INCL_GTK_HELPER_IMAGE_GRID_H
#define INCL_GTK_HELPER_IMAGE_GRID_H

#include <gtk/gtk.h>
#include "general.h"

namespace Gtk_Helper {

class Image_Grid : Gtk_Object
{
    GtkWidget *imgs_grid_widget;
    protected: GtkWidget *drawable_widget; private:

    public:
        Image_Grid()
                : imgs_grid_widget(gtk_layout_new(NULL, NULL)),
                  drawable_widget(gtk_scrolled_window_new(NULL, NULL))
        {
            gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(drawable_widget), imgs_grid_widget);
        }

        operator GtkWidget* (){ return this->drawable_widget; }


    protected:
        unsigned get_width() const
        {
            //return this->drawable_widget->allocation.height;
            return this->drawable_widget->allocation.width;
        }

        template <class T>
        void place_image(T *img, unsigned img_width, unsigned img_height,
                         unsigned horiz_pos_px, unsigned vert_pos_px)
        {
            gtk_widget_set_usize(img, img_width, img_height);
            gtk_layout_put(GTK_LAYOUT(this->imgs_grid_widget), img, horiz_pos_px, vert_pos_px);
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
