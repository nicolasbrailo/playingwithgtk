#include <iostream>
using namespace std;


#include "gtk_helper/general.h"
#include <string>


#include "ui_images.h"
#include "gtk_helper/general.h"

#include <gtk/gtk.h>
#include <vector>
using std::vector;


class Gtk_Main_Window : Gtk_Helper::Gtk_Object
{
    public:
    GtkWidget *window;

    public:
    Gtk_Main_Window() :
            window(gtk_window_new(GTK_WINDOW_TOPLEVEL)),
            autoresize_obj(NULL)
    {
        // Set style
        gtk_container_set_border_width(GTK_CONTAINER(window), 10);
        gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
        gtk_window_set_resizable(GTK_WINDOW(window), true);

        // Attach callbacks
        Gtk_Helper::connect3("delete-event", this, &Gtk_Main_Window::close_window);
        Gtk_Helper::connect2("destroy", this, &Gtk_Main_Window::quit);

        // related to return of cb method?
        Gtk_Helper::connect3("configure-event", this, &Gtk_Main_Window::resize);
    }

    operator GtkWidget* (){ return this->window; }

    void show()
    {
        gtk_widget_show_all(this->window);
    }

    template <class T>
    void add_widget(T widget)
    {
        gtk_container_add(GTK_CONTAINER(this->window), widget);
    }

    Gtk_Helper::ResizableContainer *autoresize_obj;
    void autoresize(Gtk_Helper::ResizableContainer *obj)
    {
        this->autoresize_obj = obj;
    }

    void resize()
    {
        if (this->autoresize_obj)
        {
            int width, height;
            gtk_window_get_size(GTK_WINDOW(this->window), &width, &height);
            cout << width << "x" << height << endl;
            // this->autoresize_obj->set_size(width-20, height-20);
        }
    }

    bool close_window()
    {
        return false; // Just close the window (will call quit)
    }

    void quit()
    {
        gtk_main_quit();
    }
};




#include "image_cache.h"
#include "gtk_helper/image_grid.h"

template <class UI_Image>
class Image_Grid : public Gtk_Helper::Image_Grid
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
            this->remove_image(i->get_raw_ui_ptr());
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



#include "gtk_helper/button.h"
#include "path_handler.h"

#include "gtk_helper/hbox.h"

#include <vector>


int main(int argc, char *argv[])
{
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

    Path_Handler dirs(".", &foo);


    gtk_widget_set_usize(imgs, 500, 400);
    Gtk_Helper::Gtk_HBox box(dirs, imgs);
    wnd.autoresize(&box);
    gtk_container_add(GTK_CONTAINER(wnd.window), box);
    gtk_widget_show(imgs);
    imgs.show();
    wnd.show();

    gtk_main();
    return 0;
}
