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

            Gtk_Helper::connect_raw(drawable_widget, "button-press-event", &Image_Grid::_clicked, this);
        }

        operator GtkWidget* (){ return this->drawable_widget; }


    virtual unsigned get_row_px_spacing() const = 0;
    virtual unsigned get_thumb_px_width() const = 0;
    virtual unsigned get_thumb_px_height() const = 0;

    protected:
        void clicked(unsigned x, unsigned y)
        {
            const unsigned thumb_cell_px_width = get_thumb_px_width() + get_row_px_spacing();
            const unsigned thumb_cell_px_height = get_thumb_px_height() + get_row_px_spacing();

            auto horiz_pos = x / thumb_cell_px_width;
            auto vert_pos = y / thumb_cell_px_height;

            cout << "pong " << horiz_pos << "'" << vert_pos << endl;
        }

        static void _clicked(void*, GdkEventButton* event, Image_Grid *self)
        {
            self->clicked(event->x, event->y);
        }

        unsigned get_width() const
        {
            //return this->drawable_widget->allocation.height;
            return this->drawable_widget->allocation.width;
        }

        /**
         * Put an image "somewhere" on the canvas; set the position later
         * with place_image
         */
        template <class T>
        void add_image(T *img, unsigned img_width, unsigned img_height)
        {
            gtk_widget_set_usize(img, img_width, img_height);
            gtk_layout_put(GTK_LAYOUT(this->imgs_grid_widget), img, 0, 0);
            gtk_widget_show(img);
        }

        template <class T>
        void place_image(T *img, unsigned horiz_pos_px, unsigned vert_pos_px)
        {
            gtk_layout_move(GTK_LAYOUT(this->imgs_grid_widget), img, horiz_pos_px, vert_pos_px);
        }

        template <class T>
        void remove_image(T *img)
        {
            gtk_widget_destroy(GTK_WIDGET(img));
        }
};

} /* namespace Gtk_Helper */

#endif /* INCL_GTK_HELPER_IMAGE_GRID_H */
