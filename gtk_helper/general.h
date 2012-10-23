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


    /**
     * Global behavior to make GTK a bit safer for people who don't RTFM
     */
    class Gtk_Object
    {
        public:
            // Disallow copy ctr. This is the safest if we're handling GTK
            // stuff, which many times has refcounted ptrs
            Gtk_Object(Gtk_Object&) = delete;

        protected:
            // Make no sense to instanciate this class on its own
            Gtk_Object(){}
    };
}

#endif /* INCL_GTK_HELPER_FUNCTIONS_H */
