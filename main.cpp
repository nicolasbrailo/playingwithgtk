#include <iostream>
using namespace std;


#include "gtk_helper/general.h"
#include <string>


#include "ui_images.h"

#include <gtk/gtk.h>
#include <vector>
using std::vector;
class Gtk_Main_Window
{
    public:
    GtkWidget *window;

    public:
    Gtk_Main_Window() :
            window(gtk_window_new(GTK_WINDOW_TOPLEVEL))
    {
        // Set style
        gtk_container_set_border_width(GTK_CONTAINER(window), 10);
        gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
        gtk_window_set_resizable(GTK_WINDOW(window), true);

        // Attach callbacks
        Gtk_Helper::connect(window, "delete-event", Gtk_Main_Window::close_window);
        Gtk_Helper::connect(window, "destroy", Gtk_Main_Window::quit);
    }

    // void get_size() {
    //     int width, height;
    //     gtk_window_get_size(GTK_WINDOW(window), &width, &height)
    // }

    void show()
    {
        gtk_widget_show_all(this->window);
    }

    template <class T>
    void add_widget(T widget)
    {
        gtk_container_add(GTK_CONTAINER(this->window), widget);
    }

    static gboolean close_window(GtkWidget*, GdkEvent*, gpointer)
    {
        return false; // Just close the window (will call quit)
    }

    static void quit(GtkWidget*, gpointer)
    {
        gtk_main_quit();
    }
};


#include <string>
using std::string;
class Gtk_Simple_Button
{
    public:
    GtkWidget *button;
    Gtk_Main_Window &parent;

    public:
    Gtk_Simple_Button(Gtk_Main_Window &wnd, const string &lbl) :
            button(gtk_button_new_with_label(lbl.c_str())),
            parent(wnd)
    {
        Gtk_Helper::connect(button, "clicked", Gtk_Simple_Button::clicked, this);
        wnd.add_widget(button);
    }

    private:
    static void clicked(GtkWidget* w, gpointer self)
    {
        g_print("Hello world\n");
        ((Gtk_Simple_Button*)self)->parent.quit(w, NULL);
    }
};


#include "image_cache.h"
class Gtk_Image_Grid
{
    GtkWidget *widget;

    public:
        Gtk_Image_Grid()
                : widget(gtk_layout_new(NULL, NULL))
        {}

        operator GtkWidget* (){ return this->widget; }

        unsigned get_width() const
        {
            int width, height;
            gtk_widget_get_size_request(this->widget, &width, &height);
            return width;
        }
};


class Image_Grid : public Gtk_Image_Grid
{
    vector<GtkWidget*> images;

    unsigned get_thumb_px_width() const { return 150; }
    unsigned get_row_px_height() const { return 150; }
    unsigned get_cols_per_row() const {
        return this->get_thumb_px_width() / this->get_width();
    }

    public:

    void add_widget(GtkWidget *img)
    {
        this->images.push_back(img);
    }

    void load_images()
    {
        cout << get_cols_per_row() << endl;
        const unsigned COLS_PER_ROW = 4;
        const unsigned X_SIZE = 140;
        const unsigned Y_SIZE = 140;

        for (unsigned i=0; i < images.size(); ++i)
        {
            int x = i % COLS_PER_ROW;
            int y = i / COLS_PER_ROW;
            GtkWidget *img = images[i];
            gtk_widget_set_usize(img, X_SIZE, Y_SIZE);
            GtkWidget *self = *this;
            gtk_layout_put(GTK_LAYOUT(self), img, (X_SIZE+10) * x, (Y_SIZE+10) * y);
            gtk_widget_show(img);
        }
    }
};



#include "path_handler.h"

#include "gtk_helper/hbox.h"

#include <vector>


int main(int argc, char *argv[])
{
    vector<string> files = {"img/avestruz3zv.jpg",
                            "img/no.jpg",
                            "img/squirrel_overdose.jpg",
                            "img/trained_monkey.png",
                            "img/vincent2.jpg",
                            "img/vincent3.jpg",
                            "img/vincent4.jpg",
                            "img/vincent.jpg"};

    Magick_Thumbnail_Cache cache("150");

    gtk_init(&argc, &argv);
    Gtk_Main_Window wnd;
    Image_Grid imgs;

    auto on_dir_change = [](const Path_Handler *path){
        auto files = path->get_files_on_current_dir({"jpg", "png"});
        for (auto i : files) cout << i << endl;
    };
    Path_Handler dirs("/home/nico/dev/src/playingwithgtk/img", on_dir_change);

    for (auto i : files)
    {
        const Image_Cache::Mem_Image *img = cache[i];
        Magick_Thumbnail_Cache::UI_Image_Impl ui_img(img->get_length(), img->get_buf());
        imgs.add_widget(ui_img);
    }

    //Gtk_Simple_Button btn(wnd, "Hello");
    //Gtk_Image img(wnd, "img/vincent.jpg");
    //Gtk_Image_From_PNG_Buff imaag(wnd, img.get_length(), img.get_buf());
    gtk_widget_set_usize(imgs, 500, 400);
    Gtk_Helper::Gtk_HBox box(dirs, imgs);
    gtk_container_add(GTK_CONTAINER(wnd.window), box);
    gtk_widget_show(imgs);
    wnd.show();
    imgs.load_images();

    gtk_main();
    return 0;
}
