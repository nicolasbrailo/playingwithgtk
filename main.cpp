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

#include "wget.h"



struct Scr_Img
{
    GtkWidget *img;
    int coords_x, coords_y;

    Scr_Img(const string &path, int coords_x, int coords_y)
        : img(gtk_image_new_from_file(path.c_str())),
          coords_x(coords_x), coords_y(coords_y)
    {
    }
};

#include <sstream>
#include <fstream>

#include <math.h>

struct Map_Tile_Generator
{
    // TODO: Investigate other map providers @ http://www.netmagazine.com/features/top-seven-alternatives-google-maps-api

    int zoom_level;
    int map_offset_x, map_offset_y;

    Map_Tile_Generator()
        : zoom_level(7),
          map_offset_x(64), map_offset_y(41)
    {
    }

    Scr_Img* generate_tile(int coords_x, int coords_y) const
    {
        const string& path = get_coord_path(zoom_level, coords_x, coords_y, map_offset_x, map_offset_y);
        // TODO: Check if path already exists
        if (path == "") return NULL;
        return new Scr_Img(path, coords_x, coords_y);
    }

    static void get_zoom_in_tile_coord(int tile_x, int tile_y,
                                       int *z_tile_x, int *z_tile_y)
    {
        // http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
        // Since the next zoom level will have count*2 tiles, there will be
        // 4 subtiles for each tile if zoom in one level; this means also
        // that the upper left tile in the next zoom level will be 2*coords
        *z_tile_x = tile_x * 2;
        *z_tile_y = tile_y * 2;
    }

    static void get_zoom_out_tile_coord(int tile_x, int tile_y,
                                        int *z_tile_x, int *z_tile_y)
    {
        // http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
        // Since the next zoom level will have count*2 tiles, there will be
        // 4 subtiles for each tile if zoom in one level; this means also
        // that the upper left tile in the next zoom level will be 2*coords
        *z_tile_x = tile_x / 2;
        *z_tile_y = tile_y / 2;
    }

    static void map_tile_pos_to_coords(double tile_x, double tile_y,
                                       double *coord_x, double *coord_y)
    {
        // http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
        typedef Open_Street_Map Map;
        tile_x = tile_x + Map::tile_offset_x;
        tile_y = tile_y + Map::tile_offset_y;

        unsigned zoom_level = 7;
        unsigned tiles_count = pow(2, zoom_level);

        double lon = 360 * (tile_x / tiles_count) - 180;
        double n = M_PI - 2.0 * M_PI * tile_y / tiles_count;
        double lat = 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));

        *coord_x = lon;
        *coord_y = lat;
    }

    static void map_coords_to_tile(double coord_x, double coord_y,
                                       double *tile_x, double *tile_y)
    {
        // http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
        typedef Open_Street_Map Map;

        unsigned zoom_level = 7;
        unsigned tiles_count = pow(2, zoom_level);

        *tile_x = tiles_count * (coord_x + 180) / 360;
        *tile_x = *tile_x - Map::tile_offset_x;

        *tile_y = (1.0 - log( tan(coord_y * M_PI/180.0) + 1.0 / cos(coord_y * M_PI/180.0)) / M_PI) / 2.0 * tiles_count; 
        *tile_y = *tile_y - Map::tile_offset_y;
    }



    static const string get_coord_path(int zoom, int coords_x, int coords_y, int map_offset_x, int map_offset_y)
    {
        // http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
        typedef Open_Street_Map Map;

        int tile_x = map_offset_x + coords_x;
        int tile_y = map_offset_y + coords_y;
        auto fname = Map::get_tile_fname(zoom, tile_x, tile_y);

        ifstream cached_file(fname);
        if (cached_file.good())
        {
            cout << "Got from cache " << fname << endl;
            return fname;
        }

        auto url = Map::get_tile_url(zoom, tile_x, tile_y);
        cout << "Getting " << url << " into " << fname << endl;
        wget(url, fname);
        return fname;
    }

    struct Open_Street_Map
    {
        static const int tile_offset_x = 64;
        static const int tile_offset_y = 41;

        static const string get_tile_url(int zoom, int x, int y) {
            stringstream urlss;
            urlss << "http://tile.openstreetmap.org/"<< zoom <<"/" << x << "/" << y << ".png";
            return urlss.str();
        }

        static const string get_tile_fname(int zoom, int x, int y) {
            stringstream fnamess;
            fnamess << "map/osm_" << zoom << "x" << x << "x" << y << ".png";
            return fnamess.str();
        }
    };

    struct Map_Quest
    {
        static const int tile_offset_x = 64;
        static const int tile_offset_y = 41;

        static const string get_tile_url(int zoom, int x, int y) {
            stringstream urlss;
            urlss << "http://oatile1.mqcdn.com/tiles/1.0.0/sat/" << zoom << "/" << x << "/" << y << ".jpg";
            return urlss.str();
        }

        static const string get_tile_fname(int zoom, int x, int y) {
            stringstream fnamess;
            fnamess << "map/mq_" << zoom << "x" << x << "x" << y << ".jpg";
            return fnamess.str();
        }
    };

    typedef Scr_Img UI_Tile_Image;
};


#include "scrolling_image.h"

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    Global_UI_Guard::init();
    Gtk_Main_Window wnd;

    App app;

    Map_Tile_Generator map_gen;
    Scrolling_Image<Map_Tile_Generator,
                    Scrolling_Image_Cache_Policies::Clean_Tiles_Further_Than<5>
                   > img(500, 500, map_gen);

    Gtk_Helper::Gtk_HBox box(app.dirs, Gtk_Helper::Gtk_HBox::Dont_Expand,
                             app.imgs, Gtk_Helper::Gtk_HBox::Expand,
                             img, Gtk_Helper::Gtk_HBox::Dont_Expand);
    wnd.autoresize(&app.imgs);
    wnd.add_widget(box);
    app.imgs.show();
    wnd.show();

    Global_UI_Guard ui_guard;
    gtk_main();
    return 0;

}





