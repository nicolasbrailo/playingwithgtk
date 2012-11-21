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
    public:
    // We need to define an object with a memory address for this flag
    // because we'll need to get a reference to it (see unpack)
    static const bool Expand;
    static const bool Dont_Expand;

    private:
    static const bool HOMOGENEOUS = false;
    static const unsigned SPACING = 2;

    // We must define elm as a ref, because it'll be a gtk object (otherwise
    // we'd be creating a copy of the object... not good). Since elm comes
    // packed in Tail, then all elms in Tail MUST be refs. Since all elms
    // in Tail must be refs, we need to second param (expand) to be defined
    // as ref too
    template <typename Head, typename... Tail>
    void unpack(Head &elm, const bool &expand, Tail&... rest...)
    {
        /* gboolean expand, gboolean fill, guint padding */
        gtk_box_pack_start(GTK_BOX(this->widget), elm, expand, true, 10);

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

const bool Gtk_HBox::Expand = true;
const bool Gtk_HBox::Dont_Expand = false;

} // namespace Gtk_Helper

#endif /* INCL_GTK_HELPER_HBOX_H */
