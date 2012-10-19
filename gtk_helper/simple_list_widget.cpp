#include "simple_list_widget.h"
using namespace Gtk_Helper;

#include <gtk/gtk.h>
#include "functions.h"


static void cb_row_activated(GtkTreeView*, GtkTreePath *path,
                             GtkTreeViewColumn*, gpointer self_ptr)
{
    Simple_List_Widget *self = static_cast<Simple_List_Widget*>(self_ptr);
    self->row_activated(path);
}


Simple_List_Widget::Simple_List_Widget()
        : list_store(gtk_list_store_new(1, G_TYPE_STRING)),
          view_widget(gtk_tree_view_new()),
          widget_model(GTK_TREE_MODEL(list_store))
{
    auto renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view_widget), -1, "Name", renderer, "text", 0, NULL);
    gtk_tree_view_set_model(GTK_TREE_VIEW(view_widget), widget_model);
    Gtk_Helper::connect(this->view_widget, "row-activated", cb_row_activated, this);
}


Simple_List_Widget::~Simple_List_Widget()
{
    g_object_unref(this->list_store);
}

Simple_List_Widget::operator GtkWidget* () { return this->view_widget; }


void Simple_List_Widget::load_list(const vector<string>& lst)
{
    gtk_list_store_clear(this->list_store);

    GtkTreeIter iter;
    for (auto i : lst)
    {
        const string &str = i;
        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter, 0, str.c_str(), -1);
    }

    gtk_widget_show(this->view_widget);
}


#include <iostream>
// TODO: throw up
void Simple_List_Widget::row_activated(GtkTreePath *path)
{
    GtkTreeIter iter;
    if (not gtk_tree_model_get_iter(this->widget_model, &iter, path))
    {
        std::cerr << "Error getting the element name from the list" << std::endl;
        return;
    }

    gchar *name;
    gtk_tree_model_get(this->widget_model, &iter, 0, &name, -1);
    this->element_activated(name);
    g_free(name);
}

