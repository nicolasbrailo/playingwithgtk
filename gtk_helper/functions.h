#ifndef INCL_GTK_HELPER_FUNCTIONS_H
#define INCL_GTK_HELPER_FUNCTIONS_H

#include <gtk/gtk.h>
#include <string>

namespace Gtk_Helper {
    using std::string;

    template <class Widget, typename CB>
    auto connect(Widget wdgt, const string &event, CB callback, void* data=NULL)
        -> decltype( g_signal_connect(NULL, NULL, NULL, NULL) )
    {
        return g_signal_connect(wdgt, event.c_str(), G_CALLBACK(callback), data);
    }
}

#endif /* INCL_GTK_HELPER_FUNCTIONS_H */
