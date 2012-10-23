#ifndef INCL_GTK_HELPER_SIMPLE_LIST_WIDGET_H
#define INCL_GTK_HELPER_SIMPLE_LIST_WIDGET_H

#include "general.h"

#include <string>
#include <vector>
using namespace std;

// Fwd decl of a typedef, needed for GTK types
#define GTK_FWD_DCL(cls) \
            struct _##cls; \
            typedef struct _##cls cls;

GTK_FWD_DCL(GtkWidget)
GTK_FWD_DCL(GtkListStore)
GTK_FWD_DCL(GtkTreeModel)
GTK_FWD_DCL(GtkTreePath)


namespace Gtk_Helper {

/**
 * A Simple_List_Widget wraps all GTK logic to create a TreeViewWidget,
 * including its model: no need to worry about renderers, capturing
 * events or initializing columns, it's all packed on this simple widget
 * as long as you only need a single (string) column list view.
 **/
class Simple_List_Widget : Gtk_Object
{
    GtkListStore *list_store;
    GtkWidget *view_widget;
    GtkTreeModel *widget_model;

    public:
        /**
         * Will construct an empty GtkTreeViewWidget
         * Use load_list to fill it
         */
        Simple_List_Widget();

        /**
         * Will free any used GTK resources
         */
        ~Simple_List_Widget();

        /**
         * Available to use this widget as if it were a plain GTK object
         */
        operator GtkWidget* ();

        /**
         * Subclass responsibility: will get called every time an element is
         * activated (ie double click or enter on a row) with the element
         * being activated
         */
        virtual void element_activated(const string &str) = 0;

        /**
         * Clears the list and loads lst. Will then display the list
         * on the widget (ie render it on screen)
         */
        void load_list(const vector<string>& lst);

    private:
        void row_activated(GtkTreePath *path);
};

} /* namespace Gtk_Helper */

#endif /* INCL_GTK_HELPER_SIMPLE_LIST_WIDGET_H */
