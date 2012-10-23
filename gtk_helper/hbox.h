#ifndef INCL_GTK_HELPER_HBOX_H
#define INCL_GTK_HELPER_HBOX_H

#include <gtk/gtk.h>

namespace Gtk_Helper {

class Gtk_HBox
{
    static const bool HOMOGENEOUS = false;
    static const unsigned SPACING = 2;

    public:
        operator GtkWidget* (){ return this->widget; }

        // TODO: Looks great for variadic tmpls
        Gtk_HBox(GtkWidget* x, GtkWidget* y)
                : widget(gtk_hbox_new(HOMOGENEOUS, SPACING))
        {
            /* gboolean expand, gboolean fill, guint padding */
            gtk_box_pack_start(GTK_BOX(this->widget), x, false, false, 0);
            gtk_box_pack_start(GTK_BOX(this->widget), y, false, false, 0);
        }

    private:
        GtkWidget *widget;
};

} // namespace Gtk_Helper

#endif /* INCL_GTK_HELPER_HBOX_H */
