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


    protected:
        unsigned get_width() const
        {
            int width, height;
            gtk_widget_get_size_request(this->widget, &width, &height);
            return width;
        }

        template <class T>
        void place_image(T *img, unsigned img_width, unsigned img_height,
                         unsigned horiz_pos_px, unsigned vert_pos_px)
        {
            gtk_widget_set_usize(img, img_width, img_height);
            gtk_layout_put(GTK_LAYOUT(this->widget), img, horiz_pos_px, vert_pos_px);
            gtk_widget_show(img);
        }
};


template <class UI_Image>
class Image_Grid : public Gtk_Image_Grid
{
    vector<UI_Image*> images;

    unsigned get_row_px_spacing() const { return 5; }
    unsigned get_thumb_px_width() const { return 150; }
    unsigned get_thumb_px_height() const { return 150; }
    unsigned get_cols_per_row() const {
        return this->get_width() / this->get_thumb_px_width();
    }

    public:
    void add_widget(const Image_Cache::Mem_Image *img)
    {
        // TODO: should just forward Image_Cache::Mem_Image?
        this->images.push_back(
                new UI_Image(img->get_length(), img->get_buf())
        );
    }

    void clear()
    {
        for (auto i : images)
        {
            gtk_widget_destroy(GTK_WIDGET(i->get_raw_ui_ptr()));
            delete i;
        }

        images.clear();
    }

    void show()
    {
        unsigned cnt_imgs = images.size();
        for (unsigned i=0; i < cnt_imgs; ++i)
        {
            // Meassure of the cell containing a thumb
            const unsigned thumb_cell_px_width = get_thumb_px_width() + get_row_px_spacing();
            const unsigned thumb_cell_px_height = get_thumb_px_height() + get_row_px_spacing();

            // (x,y) position of the image, where the position is the number
            // of the cell it occupies (imagine an excel spreadsheet)
            const unsigned horiz_pos = i % this->get_cols_per_row();
            const unsigned vert_pos = i / this->get_cols_per_row();

            // Converting the (x,y) position to (x,y) pixel position
            const unsigned horiz_pos_px = horiz_pos * thumb_cell_px_width;
            const unsigned vert_pos_px = vert_pos * thumb_cell_px_height;

            this->place_image(images[i]->get_raw_ui_ptr(),
                            this->get_thumb_px_width(), this->get_thumb_px_height(),
                            horiz_pos_px, vert_pos_px);
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

    gtk_init(&argc, &argv);
    Gtk_Main_Window wnd;

    Magick_Thumbnail_Cache cache("150");
    Image_Grid<Magick_Thumbnail_Cache::UI_Image_Impl> imgs;

    struct Foo : public Path_Handler::Dir_Changed_CB
    {
        Image_Grid<Magick_Thumbnail_Cache::UI_Image_Impl> *imgs;
        Magick_Thumbnail_Cache *cache;

        Foo(Image_Grid<Magick_Thumbnail_Cache::UI_Image_Impl> *imgs, Magick_Thumbnail_Cache *cache)
            : imgs(imgs), cache(cache)
        {}

        void on_dir_changed(const Path_Handler *path)
        {
            imgs->clear();

            auto files = path->get_files_on_current_dir({"jpg", "png"});
            auto cwd = path->get_current_path();
            for (auto i : files)
            {
                Magick_Thumbnail_Cache &rcache = *cache;
                auto img_abs_path = cwd + '/' + i;
                // TODO: Render unrendereable imgs with a broken icon
                const Image_Cache::Mem_Image *img = rcache[img_abs_path];
                imgs->add_widget(img);
            }

            imgs->show();
        }
    } foo(&imgs, &cache);

    Path_Handler dirs("/home/nico/dev/src/playingwithgtk/img", &foo);

    for (auto i : files)
    {
        const Image_Cache::Mem_Image *img = cache[i];
        imgs.add_widget(img);
    }

    //Gtk_Simple_Button btn(wnd, "Hello");
    //Gtk_Image img(wnd, "img/vincent.jpg");
    //Gtk_Image_From_PNG_Buff imaag(wnd, img.get_length(), img.get_buf());
    gtk_widget_set_usize(imgs, 500, 400);
    Gtk_Helper::Gtk_HBox box(dirs, imgs);
    gtk_container_add(GTK_CONTAINER(wnd.window), box);
    gtk_widget_show(imgs);
    imgs.show();
    wnd.show();

    gtk_main();
    return 0;
}
