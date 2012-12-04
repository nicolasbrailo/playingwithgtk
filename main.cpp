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
    int abs_x, abs_y;

    Scr_Img(const string &path, int abs_x, int abs_y)
        : img(gtk_image_new_from_file(path.c_str())),
          abs_x(abs_x), abs_y(abs_y)
    {
    }
};

#include <sstream>

map<long, string> map_tile_cache;

struct Map_Tile_Generator {
    static Scr_Img* generate_tile(int abs_x, int abs_y)
    {
        auto it = map_tile_cache.find(abs_x * 1000 + abs_y);
        if (it != map_tile_cache.end()) return new Scr_Img(it->second, abs_x, abs_y);

        const string& path = get_coord_path(abs_x, abs_y);
        if (path == "") return NULL;

        map_tile_cache[abs_x * 1000 + abs_y] = path;
        return new Scr_Img(path, abs_x, abs_y);
    }

    static const string get_coord_path(int abs_x, int abs_y)
    {
        typedef Map_Quest Map;

        int tile_x = Map::tile_offset_x + abs_x;
        int tile_y = Map::tile_offset_y + abs_y;
        auto url = Map::get_tile_url(tile_x, tile_y);
        auto fname = Map::get_tile_fname(tile_x, tile_y);

        wget(url, fname);
        return fname;
    }

    struct Open_Street_Map
    {
        static const int tile_offset_x = 64;
        static const int tile_offset_y = 41;

        static const string get_tile_url(int x, int y) {
            stringstream urlss;
            urlss << "http://tile.openstreetmap.org/7/" << x << "/" << y << ".png";
            return urlss.str();
        }

        static const string get_tile_fname(int x, int y) {
            stringstream fnamess;
            fnamess << "map/img" << x << "x" << y << ".png";
            return fnamess.str();
        }
    };

    struct Map_Quest
    {
        static const int tile_offset_x = 64;
        static const int tile_offset_y = 41;

        static const string get_tile_url(int x, int y) {
            stringstream urlss;
            urlss << "http://oatile1.mqcdn.com/tiles/1.0.0/sat/07/" << x << "/" << y << ".jpg";
            return urlss.str();
        }

        static const string get_tile_fname(int x, int y) {
            stringstream fnamess;
            fnamess << "map/img" << x << "x" << y << ".png";
            return fnamess.str();
        }
    };

    typedef Scr_Img UI_Tile_Image;
};


#include "gtk_helper/mouse_draggable.h"

template <class Tile_Generator>
class Scrolling_Image : Gtk_Helper::Mouse_Draggable<5>
{
    struct Point {
        int x, y;
        Point(int x, int y) : x(x), y(y) {}

        bool operator < (const Point& pt) const {
            if (x == pt.x) return y < pt.y;
            return x < pt.x;
        }

        const Point& operator -= (const Point &p) { x -= p.x; y -= p.y; return *this; }
        const Point& operator += (const Point &p) { x += p.x; y += p.y; return *this; }
        Point operator + (const Point &p) const { return Point(x + p.x, y + p.y); }
        Point operator - (const Point &p) const { return Point(x - p.x, y - p.y); }
        Point operator % (const Point &p) const { return Point(x % p.x, y % p.y); }
        Point operator + (int n) const { return Point(x + n, y + n); }
        Point operator - (int n) const { return Point(x - n, y - n); }
        Point operator * (int n) const { return Point(x * n, y * n); }
        Point operator % (int n) const { return Point(x % n, y % n); }
    };

    public:

    void mouse_dragged(int dx, int dy)
    {
        current_pos -= Point(dx, dy);
        this->update_things();
    }

    GtkWidget *canvas_window;

    int tile_height, tile_width;
    int tiles_to_cache;

    Point current_pos;

    typedef typename Tile_Generator::UI_Tile_Image UI_Tile_Image;

    vector<UI_Tile_Image*> tiles;

    Scrolling_Image(unsigned default_height, unsigned default_width)
            : Mouse_Draggable::Mouse_Draggable(gtk_layout_new(NULL, NULL)),
              tile_height(256), tile_width(256), tiles_to_cache(3),
              current_pos(0, 0)
    {
        canvas_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(canvas_window), this->ui_widget());

        gtk_widget_set_usize(this->ui_widget(), default_height, default_width);
        gtk_widget_set_usize(canvas_window, default_height, default_width);

        this->update_things();
    }

    void update_things()
    {
        for (auto tile : tiles) {
            gtk_layout_move(GTK_LAYOUT(this->ui_widget()), tile->img, -1000, -1000);
            //gtk_widget_destroy(GTK_WIDGET(tile->img));
            //delete tile;
        }
        //tiles.clear();

        // Moding with the tile box, we can align the current pos to a grid
        // determined by the tile width and height; eg if each tile is 10x20
        // and curr pos is 35x42, then curr_pos is 5x2 inside a tile and the
        // closest point in the grid determined by the tiles' size will be
        // 30x40. Then, grid_point = curr_pos - (curr_pos % tile size)
        Point tile_area(tile_width, tile_height);
        Point curr_grid_point = current_pos - (current_pos % tile_area);

        // We'll start preloading N tiles - and + from the current
        // grid point
        Point preload_start = curr_grid_point - (tile_area * tiles_to_cache);
        Point preload_end = curr_grid_point + (tile_area * tiles_to_cache);

        // Start going through every square in the grid and get a tile for it
        for (int x = preload_start.x; x < preload_end.x; x += tile_width) {
            for (int y = preload_start.y; y < preload_end.y; y += tile_height) {
                // Current position of the grid square we're rendering
                Point grid_square(x, y);

                // The current_pos and the grid pos are virtual measures
                // not related to the currently on-screen tiles: the diff
                // between the grid square and the current pos will give us
                // the physical offset in which to place the image. Eg: If
                // we are in position (42, 24) and we're loading the tile at
                // (30, 10) then we should render the tile at (12, 14)
                Point phys_offset = grid_square - current_pos;

                // The coords for the tile in the grid
                Point tile_coords(x, y);

                // Take care of the real rendering
                render_tile(phys_offset, tile_coords);
            }
        }
    }

    map<Point, UI_Tile_Image*> tiles_cache;
    void render_tile(const Point &tile_render_point, const Point &tile_coords)
    {
        auto it = tiles_cache.find(tile_coords);
        if (it != tiles_cache.end())
        {
            auto img_widget = it->second;
            gtk_layout_move(GTK_LAYOUT(this->ui_widget()), img_widget->img, tile_render_point.x, tile_render_point.y);
            gtk_widget_show(img_widget->img);

        } else {
            // Get the tile for the square x,y
            int mapped_coords_x = tile_coords.x / tile_width;
            int mapped_coords_y = tile_coords.y / tile_width;
            auto img = Tile_Generator::generate_tile(mapped_coords_x, mapped_coords_y);
            if (img) {
                tiles.push_back(img);
                tiles_cache[tile_render_point] = img;

                gtk_layout_put(GTK_LAYOUT(this->ui_widget()), img->img, tile_render_point.x, tile_render_point.y);
                gtk_widget_show(img->img);
            }
        }
    }
};


int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    Global_UI_Guard::init();
    Gtk_Main_Window wnd;

    App app;
    Scrolling_Image<Map_Tile_Generator> img(500, 500);

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





