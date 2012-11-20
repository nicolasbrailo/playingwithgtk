#ifndef INCL_GTK_HELPER_BUTTON_H
#define INCL_GTK_HELPER_BUTTON_H

#include <gtk/gtk.h>
#include "general.h"

#include <string>

namespace Gtk_Helper {
using std::string;

class Button : Gtk_Object
{
    public:
    GtkWidget *widget;

    public:
        operator GtkWidget* (){ return this->widget; }

        Button(const string &lbl) :
                widget(gtk_button_new_with_label(lbl.c_str()))
        {
            Gtk_Helper::connect3("clicked", this, &Button::clicked);
        }

        //virtual void clicked() = 0;
        void clicked(){
            std::cout << "HOLA" << std::endl;
        }
};

} /* namespace Gtk_Helper */

#endif /* INCL_GTK_HELPER_BUTTON_H */
