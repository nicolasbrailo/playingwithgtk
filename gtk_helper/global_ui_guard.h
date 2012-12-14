#ifndef INCL_GTK_HELPER_GLOBAL_UI_GUARD_H
#define INCL_GTK_HELPER_GLOBAL_UI_GUARD_H

#include <gtk/gtk.h>
struct Global_UI_Guard
{
    Global_UI_Guard() { gdk_threads_enter(); }
    ~Global_UI_Guard() { gdk_threads_leave(); }

    static void init() { gdk_threads_init(); }
};

#endif /* INCL_GTK_HELPER_GLOBAL_UI_GUARD_H */
