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

#include "wget.h"



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

#include <sstream>
const string get_coord_path(int abs_x, int abs_y)
{
    // Start centered near Ams
    int tile_x = 64 + abs_x;
    int tile_y = 43 + abs_y;
    stringstream urlss, fnamess;
    urlss << "http://tile.openstreetmap.org/7/" << tile_x << "/" << tile_y << ".png";
    fnamess << "map/img" << tile_x << "x" << tile_y << ".png";
    string url = urlss.str();
    string fname = fnamess.str();
    cout << "Loading " << url << endl;

    wget(url, fname);
    return fname;
}

map<long, string> map_tile_cache;

Scr_Img* mk_scr_img(int abs_x, int abs_y, int width, int height)
{
    abs_x = abs_x / 256;
    abs_y = abs_y / 256;
    auto it = map_tile_cache.find(abs_x * 1000 + abs_y);
    if (it != map_tile_cache.end()) return new Scr_Img(it->second, abs_x, abs_y, width, height);

    const string& path = get_coord_path(abs_x, abs_y);
    if (path == "") return NULL;

    map_tile_cache[abs_x * 1000 + abs_y] = path;
    return new Scr_Img(path, abs_x, abs_y, width, height);
}


struct Scrolling_Image
{
    GtkWidget *canvas;
    GtkWidget *canvas_window;

    int tile_height;
    int tile_width;

    int tiles_to_cache;

    int canvas_width;
    int canvas_height;

    int current_pos_x, current_pos_y;

    vector<Scr_Img*> tiles;

    Scrolling_Image()
            : tile_height(256), tile_width(256),
              canvas_height(512), canvas_width(300),
              tiles_to_cache(3),
              current_pos_x(0), current_pos_y(0)
    {
        canvas = gtk_layout_new(NULL, NULL);
        canvas_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(canvas_window), canvas);

        gtk_widget_add_events(GTK_WIDGET(canvas), GDK_BUTTON_RELEASE_MASK);
        gtk_widget_add_events(GTK_WIDGET(canvas), GDK_BUTTON_PRESS_MASK);

        Gtk_Helper::connect_raw(canvas, "button-press-event", &Scrolling_Image::_clicked, this);
        Gtk_Helper::connect_raw(canvas, "button-release-event", &Scrolling_Image::_released, this);

        gtk_widget_set_usize(canvas, canvas_height, canvas_width);
        gtk_widget_set_usize(canvas_window, canvas_height, canvas_width);


        update_things();
    }

    void update_things()
    {
        for (auto tile : tiles) {
            gtk_widget_destroy(GTK_WIDGET(tile->img));
            delete tile;
        }
        tiles.clear();

        int min_x = current_pos_x - (current_pos_x % tile_width);
        int min_y = current_pos_y - (current_pos_y % tile_width);

        int preload_x_start = min_x - tiles_to_cache * tile_width;
        int preload_y_start = min_y - tiles_to_cache *tile_height;
        int preload_x_end = min_x + tiles_to_cache *tile_width;
        int preload_y_end = min_y + tiles_to_cache *tile_height;

        for (int x = preload_x_start; x < preload_x_end; x += tile_width) {
            for (int y = preload_y_start; y < preload_y_end; y += tile_height) {
                auto img = mk_scr_img(x, y, tile_width, tile_height);
                if (img) {
                    tiles.push_back(img);

                    int place_x = x - current_pos_x;
                    int place_y = y - current_pos_y;
                    gtk_layout_put(GTK_LAYOUT(canvas), img->img, place_x, place_y);
                    gtk_widget_show(img->img);
                }
            }
        }
    }

    int click_start_x, click_start_y;
    void clicked(int x, int y) { click_start_x = x; click_start_y = y; }
    void released(int x, int y) {
        int dx = x - click_start_x;
        int dy = y - click_start_y; 
        current_pos_x -= dx;
        current_pos_y -= dy;
        update_things();
    }

    static void _clicked(void*, GdkEventButton* event, Scrolling_Image *self)
        { self->clicked(event->x, event->y); }
    static void _released(void*, GdkEventButton* event, Scrolling_Image *self)
        { self->released(event->x, event->y); }

};


int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    Global_UI_Guard::init();
    Gtk_Main_Window wnd;

    App app;
    Scrolling_Image img;

    Gtk_Helper::Gtk_HBox box(app.dirs, Gtk_Helper::Gtk_HBox::Dont_Expand,
                             app.imgs, Gtk_Helper::Gtk_HBox::Expand,
                             img.canvas_window, Gtk_Helper::Gtk_HBox::Dont_Expand);
    wnd.autoresize(&app.imgs);
    wnd.add_widget(box);
    app.imgs.show();
    wnd.show();

    Global_UI_Guard ui_guard;
    gtk_main();
    return 0;

}





