#ifndef INCL_GTK_HELPER_HBOX_H
#define INCL_GTK_HELPER_HBOX_H

#include <gtk/gtk.h>
#include "general.h"

namespace Gtk_Helper {

/**
 * A wrapper for a GTK HBox with a variadic constructor, so we can quickly
 * and easily build a box with as many elements as we need
 */
class Gtk_HBox : Gtk_Object
{
    static const bool HOMOGENEOUS = false;
    static const unsigned SPACING = 2;

    template <typename Head, typename... Tail>
    void unpack(Head &elm, Tail&... rest...)
    {
        /* gboolean expand, gboolean fill, guint padding */
        gtk_box_pack_start(GTK_BOX(this->widget), elm, false, false, 0);

        unpack(rest...);
    }

    // End condition
    void unpack(){}

    public:
        operator GtkWidget* (){ return this->widget; }

        template <typename... Pack_List>
        Gtk_HBox(Pack_List&... widgets...)
                : widget(gtk_hbox_new(HOMOGENEOUS, SPACING))
        {
            unpack(widgets...);
        }

    private:
        GtkWidget *widget;
};

} // namespace Gtk_Helper

#endif /* INCL_GTK_HELPER_HBOX_H */
