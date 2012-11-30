#include <iostream>
#include <time.h>
using namespace std;


#include "gtk_helper/general.h"
#include <string>


#include <gtk/gtk.h>
#include <vector>
using std::vector;


typedef int Gdk_Evt_Processing;
const Gdk_Evt_Processing GDK_SHOULD_CONTINUE_PROCESSING = 0;
const Gdk_Evt_Processing GDK_SHOULD_STOP_PROCESSING = 0;


struct Global_UI_Guard
{
    Global_UI_Guard() { gdk_threads_enter(); }
    ~Global_UI_Guard() { gdk_threads_leave(); }

    static void init() { gdk_threads_init(); }
};

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
        Gtk_Helper::connect3("configure-event", this, &Gtk_Main_Window::resize);
    }

    operator GtkWidget* (){ return this->window; }

    void show()
    {
        gtk_widget_show_all(this->window);
    }

    template <class T>
    void add_widget(T &widget)
    {
        gtk_container_add(GTK_CONTAINER(this->window), widget);
    }

    Gtk_Helper::ResizableContainer *autoresize_obj;
    void autoresize(Gtk_Helper::ResizableContainer *obj)
    {
        this->autoresize_obj = obj;
    }

    Gdk_Evt_Processing resize()
    {
        if (this->autoresize_obj)
        {
            //int width, height;
            //gtk_window_get_size(GTK_WINDOW(this->window), &width, &height);
            this->autoresize_obj->resize();
        }

        return GDK_SHOULD_CONTINUE_PROCESSING;
    }

    Gdk_Evt_Processing close_window()
    {
        return GDK_SHOULD_CONTINUE_PROCESSING;
    }

    void quit()
    {
        gtk_main_quit();
    }
};



#include <thread>


#include "image.h"
#include "deferred_image_loader.h"

#include "image_cache.h"
#include "gtk_helper/image_grid.h"

template <class UI_Image>
class Image_Grid : public Gtk_Helper::Image_Grid, public Gtk_Helper::ResizableContainer
{
    vector<UI_Image*> images;

    unsigned get_row_px_spacing() const { return 5; }
    unsigned get_thumb_px_width() const { return 150; }
    unsigned get_thumb_px_height() const { return 150; }
    unsigned get_cols_per_row() const {
        auto n = this->get_width() / this->get_thumb_px_width();
        return n < 1? 1 : n;
    }

    public:
    template <class Cache>
    void add_widget(Cache &cache, const string &path)
    {
        auto ui_img = new UI_Image(path, "./empty.png");
        cache.process(ui_img);

        this->images.push_back(ui_img);
        // Add to canvas
        this->add_image(ui_img->get_raw_ui_ptr(),
                        this->get_thumb_px_width(), this->get_thumb_px_height());
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

    void resize()
    {
        this->show();
    }

    void show()
    {
        // Meassure of the cell containing a thumb
        const unsigned thumb_cell_px_width = get_thumb_px_width() + get_row_px_spacing();
        const unsigned thumb_cell_px_height = get_thumb_px_height() + get_row_px_spacing();
        const unsigned cols_per_row = this->get_cols_per_row();

        unsigned cnt_imgs = images.size();
        for (unsigned i=0; i < cnt_imgs; ++i)
        {
            // (x,y) position of the image, where the position is the number
            // of the cell it occupies (imagine an excel spreadsheet)
            const unsigned horiz_pos = i % cols_per_row;
            const unsigned vert_pos = i / cols_per_row;

            // Converting the (x,y) position to (x,y) pixel position
            const unsigned horiz_pos_px = horiz_pos * thumb_cell_px_width;
            const unsigned vert_pos_px = vert_pos * thumb_cell_px_height;

            this->place_image(images[i]->get_raw_ui_ptr(), horiz_pos_px, vert_pos_px);
        }
    }
};


#include "path_handler.h"
struct App : public Path_Handler::Dir_Changed_CB
{
    Image_Cache cache;
    Image_Grid< Image > imgs;
    Deferred_Image_Loader<Image_Cache, Image> def_proc;
    Path_Handler dirs;

    App()
        : def_proc(cache, 5),
          dirs(".", this)
    {}

    void on_dir_changed(const Path_Handler *path)
    {
        imgs.clear();

        auto files = path->get_files_on_current_dir({"jpg", "png"});
        auto cwd = path->get_current_path();
        for (auto i : files)
        {
            auto img_abs_path = cwd + '/' + i;
            imgs.add_widget(def_proc, img_abs_path);
        }

        imgs.show();
    }
};




#include "gtk_helper/button.h"
#include "gtk_helper/hbox.h"

int main2(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    Global_UI_Guard::init();
    Gtk_Main_Window wnd;

    App app;

    Gtk_Helper::Gtk_HBox box(app.dirs, Gtk_Helper::Gtk_HBox::Dont_Expand,
                             app.imgs, Gtk_Helper::Gtk_HBox::Expand);
    wnd.autoresize(&app.imgs);
    wnd.add_widget(box);
    app.imgs.show();
    wnd.show();

    Global_UI_Guard ui_guard;
    gtk_main();
    return 0;
}



struct Scr_Img
{
    GtkWidget *img;
    unsigned width, height;
    int abs_x, abs_y;

    Scr_Img(const string &path, int abs_x, int abs_y, int width, int height)
        : img(gtk_image_new_from_file(path.c_str())),
          width(width), height(height),
          abs_x(abs_x), abs_y(abs_y)
    {
        gtk_widget_set_usize(img, width, height);
    }
};

struct Scrolling_Image
{
    GtkWidget *canvas;
    GtkWidget *canvas_window;

    int tile_height;
    int tile_width;

    int current_pos_x, current_pos_y;

    vector<Scr_Img*> tiles;

    Scrolling_Image()
            : tile_height(256), tile_width(256),
              current_pos_x(0), current_pos_y(0)
    {
        canvas = gtk_layout_new(NULL, NULL);
        canvas_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(canvas_window), canvas);

        gtk_widget_add_events(GTK_WIDGET(canvas), GDK_BUTTON_RELEASE_MASK);
        gtk_widget_add_events(GTK_WIDGET(canvas), GDK_BUTTON_PRESS_MASK);

        Gtk_Helper::connect_raw(canvas, "button-press-event", &Scrolling_Image::_clicked, this);
        Gtk_Helper::connect_raw(canvas, "button-release-event", &Scrolling_Image::_released, this);

        gtk_widget_set_usize(canvas, 512, 512);
        gtk_widget_set_usize(canvas_window, 512, 512);


        auto img1 = new Scr_Img("./map/img1.png", 0,   0, tile_width, tile_height);
        auto img2 = new Scr_Img("./map/img2.png", 256, 0, tile_width, tile_height);
        auto img3 = new Scr_Img("./map/img3.png", 512, 0, tile_width, tile_height);
        auto img4 = new Scr_Img("./map/img4.png", 0,   256, tile_width, tile_height);
        auto img5 = new Scr_Img("./map/img5.png", 256, 256, tile_width, tile_height);
        auto img6 = new Scr_Img("./map/img6.png", 512, 256, tile_width, tile_height);
        auto img7 = new Scr_Img("./map/img7.png", 0,   512, tile_width, tile_height);
        auto img8 = new Scr_Img("./map/img8.png", 256, 512, tile_width, tile_height);
        auto img9 = new Scr_Img("./map/img9.png", 512, 512, tile_width, tile_height);

        tiles.push_back(img1);
        tiles.push_back(img2);
        tiles.push_back(img3);
        tiles.push_back(img4);
        tiles.push_back(img5);
        tiles.push_back(img6);
        tiles.push_back(img7);
        tiles.push_back(img8);
        tiles.push_back(img9);

        gtk_layout_put(GTK_LAYOUT(canvas), img1->img, -1000, -1000);
        gtk_layout_put(GTK_LAYOUT(canvas), img2->img, -1000, -1000);
        gtk_layout_put(GTK_LAYOUT(canvas), img3->img, -1000, -1000);
        gtk_layout_put(GTK_LAYOUT(canvas), img4->img, -1000, -1000);
        gtk_layout_put(GTK_LAYOUT(canvas), img5->img, -1000, -1000);
        gtk_layout_put(GTK_LAYOUT(canvas), img6->img, -1000, -1000);
        gtk_layout_put(GTK_LAYOUT(canvas), img7->img, -1000, -1000);
        gtk_layout_put(GTK_LAYOUT(canvas), img8->img, -1000, -1000);
        gtk_layout_put(GTK_LAYOUT(canvas), img9->img, -1000, -1000);

        foo();
    }

    int click_start_x, click_start_y;
    void clicked(int x, int y) { click_start_x = x; click_start_y = y; }
    void released(int x, int y) {
        int dx = x - click_start_x;
        int dy = y - click_start_y; 
        current_pos_x += dx;
        current_pos_y += dy;
        foo();
    }

    static void _clicked(void*, GdkEventButton* event, Scrolling_Image *self)
        { self->clicked(event->x, event->y); }
    static void _released(void*, GdkEventButton* event, Scrolling_Image *self)
        { self->released(event->x, event->y); }

    void foo()
    {
        int current_pos_end_x = current_pos_x + 400;
        int current_pos_end_y = current_pos_y + 400;

        for (auto tile : tiles)
        {
            bool in_x_range = tile->abs_x > current_pos_x or tile->abs_x < current_pos_end_x;
            bool in_y_range = tile->abs_y > current_pos_y or tile->abs_y < current_pos_end_y;

            if (in_x_range and in_y_range)
            {
                int x_tile_pos = tile->abs_x - current_pos_x;
                int y_tile_pos = tile->abs_y - current_pos_y;
                gtk_layout_move(GTK_LAYOUT(canvas), tile->img, x_tile_pos, y_tile_pos);
            }
        }
    }
};


int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    Global_UI_Guard::init();
    Gtk_Main_Window wnd;

    Scrolling_Image img;

    wnd.add_widget(img.canvas_window);
    wnd.show();

    Global_UI_Guard ui_guard;
    gtk_main();
    return 0;
}




















