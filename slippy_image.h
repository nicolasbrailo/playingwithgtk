#ifndef INCL_SCROLLING_IMAGE_H
#define INCL_SCROLLING_IMAGE_H

#include "gtk_helper/slippy_controller.h"

template <class Tile_Generator, class Cache_Clean_Up_Policy>
class Slippy_Image : public Gtk_Helper::Slippy_Image<5>
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

    void mouse_dragged(int dx, int dy)
    {
        current_pos -= Point(dx, dy);
        this->update_tiles();
    }

    void mouse_scrolled(bool scroll_up, int x, int y)
    {
        const int tile_width = tile_generator.get_tile_width();
        const int tile_height = tile_generator.get_tile_height();

        double click_coords_x = 1.0 * (x + current_pos.x) / tile_width;
        double click_coords_y = 1.0 * (y + current_pos.y) / tile_height;

        double pos_x = tile_width * (click_coords_x - (int)click_coords_x);
        double pos_y = tile_height * (click_coords_y - (int)click_coords_y);
        current_pos = Point(pos_x, pos_y);

        if (scroll_up) {
            tile_generator.zoom_in(click_coords_x, click_coords_y);
        } else {
            tile_generator.zoom_out(click_coords_x, click_coords_y);
        }


        cout << "Asking all images to be deleted" << endl;
        for (auto tile : all_known_tiles)
            tile_generator.destroy(tile);

        tile_coords_cache.clear();
        all_known_tiles.clear();

        cout << "Done. Start tiles upd" << endl;

        update_tiles();
    }

    void mouse_clicked(int x, int y)
    {
        const int tile_width = tile_generator.get_tile_width();
        const int tile_height = tile_generator.get_tile_height();

        // click_point + current_pos = absolute (physical) offset in the map
        // offset / tile_area = click in map coords (int part is tile number,
        // float part is offset into tile)
        double click_coords_x = 1.0 * (x + current_pos.x) / tile_width;
        double click_coords_y = 1.0 * (y + current_pos.y) / tile_height;

        double real_coords_x, real_coords_y;
        tile_generator.map_tile_pos_to_coords(click_coords_x, click_coords_y,
                                               &real_coords_x, &real_coords_y);

        cout << "Clicked " << real_coords_x << "x" << real_coords_y << endl;

        double xx, yy;
        tile_generator.map_coords_to_tile(real_coords_x, real_coords_y, &xx, &yy);
        cout << "Clicked " << real_coords_x << "x" << real_coords_y
             << " == tile(" << xx << "x" << yy << ")" << endl;
    }


    Tile_Generator &tile_generator;

    int tiles_to_prefetch;

    Point current_pos;

    typedef typename Tile_Generator::UI_Tile_Image UI_Tile_Image;

    map<Point, UI_Tile_Image*> tile_coords_cache;
    vector<UI_Tile_Image*> all_known_tiles;

    public:

    Slippy_Image(unsigned default_width, unsigned default_height, Tile_Generator &tile_generator)
            : tile_generator(tile_generator),
              tiles_to_prefetch(3),
              current_pos(0, 0)
    {
        this->set_size(default_width, default_height);
        this->update_tiles();
    }

    private:
    void update_tiles()
    {
        const int tile_width = tile_generator.get_tile_width();
        const int tile_height = tile_generator.get_tile_height();

        // Moding with the tile box, we can align the current pos to a grid
        // determined by the tile width and height; eg if each tile is 10x20
        // and curr pos is 35x42, then curr_pos is 5x2 inside a tile and the
        // closest point in the grid determined by the tiles' size will be
        // 30x40. Then, grid_point = curr_pos - (curr_pos % tile size)
        Point tile_area(tile_width, tile_height);
        Point upper_left_tile_pos = current_pos - (current_pos % tile_area);

        // The coordinates of the tile in the upper left corner of the grid
        // these coords are not relative to the control, they are relative to
        // the map itself. In the example above we would be rendering the tile
        // at coords 3x2 (upper_left_tile_pos = 30x40, tile_area = 10x20)
        Point upper_left_tile_coords = Point(upper_left_tile_pos.x / tile_width,
                                           upper_left_tile_pos.y / tile_height);

        // We'll start preloading N tiles - and + from the current
        // grid point
        Point preload_start = upper_left_tile_pos - (tile_area * tiles_to_prefetch);
        Point preload_end = upper_left_tile_pos + (tile_area * tiles_to_prefetch);

        // Give the chance of the cleanup policy to do something before rendering
        Cache_Clean_Up_Policy::clean_up_pre_render(all_known_tiles,
                            upper_left_tile_coords.x, upper_left_tile_coords.y);

        // Start going through every square in the grid and get a tile for it
        for (int x = preload_start.x; x < preload_end.x; x += tile_width) {
            for (int y = preload_start.y; y < preload_end.y; y += tile_height) {
                // Current position of the grid square we're rendering
                Point rendering_tile_grid_pos(x, y);

                // The current_pos and the grid pos are virtual measures
                // not related to the currently on-screen tiles: the diff
                // between the grid square and the current pos will give us
                // the physical offset in which to place the image. Eg: If
                // we are in position (42, 24) and we're loading the tile at
                // (30, 10) then we should render the tile at (12, 14)
                Point phys_offset = rendering_tile_grid_pos - current_pos;

                // The coords for the tile in the grid
                Point tile_coords(x / tile_width, y / tile_height);

                // Take care of the real rendering
                render_tile(phys_offset, tile_coords);
            }
        }

        // Each tile "knows" its coords but we need to find it's distance to
        // the rendering offset, not to the real map, so we need to add the
        // map offset to our current location to match the tile coords
        // (It's easier to displace or coords instead of the coords of each tile)
        auto map_pos_x = upper_left_tile_coords.x + tile_generator.get_offset_x();
        auto map_pos_y = upper_left_tile_coords.y + tile_generator.get_offset_y();
        Cache_Clean_Up_Policy::clean_up_post_render(all_known_tiles, map_pos_x, map_pos_y);
    }

    void render_tile(const Point &tile_render_point, const Point &tile_coords)
    {
        auto it = tile_coords_cache.find(tile_coords);
        if (it != tile_coords_cache.end())
        {
            auto img_widget = it->second;
            this->move_image(img_widget, tile_render_point.x, tile_render_point.y);
        } else {
            // Get the tile for the square x,y
            auto img = tile_generator.generate_tile(tile_coords.x, tile_coords.y);
            if (img) {
                tile_coords_cache[tile_coords] = img;
                all_known_tiles.push_back(img);
                this->place_image(img, tile_render_point.x, tile_render_point.y);
            }
        }
    }
};

struct Scrolling_Image_Cache_Policies
{
    struct Never_Clean {
        template <class Lst> static void clean_up_pre_render(Lst, int, int){}
        template <class Lst> static void clean_up_post_render(Lst, int, int){}
    };

    struct No_Cache {
        template <class Lst> static void clean_up_pre_render(Lst tiles, int, int) {
            // TODO: Delete all tiles
        }
        template <class Lst> static void clean_up_post_render(Lst tiles, int, int){}
    };

    template <int tiles_threshold>
    struct Clean_Tiles_Further_Than
    {
        template <class Lst>
        static void clean_up_post_render(Lst tiles, int pos_x, int pos_y)
        {
            int min_grid_coord_x = pos_x - tiles_threshold;
            int min_grid_coord_y = pos_y - tiles_threshold;
            int max_grid_coord_x = pos_x + tiles_threshold;
            int max_grid_coord_y = pos_y + tiles_threshold;

            for (auto tile : tiles)
            {
                if (tile->coords_x < min_grid_coord_x or
                        tile->coords_x > max_grid_coord_x or
                        tile->coords_y < min_grid_coord_y or
                        tile->coords_y > max_grid_coord_y)
                {
                    cout << "I should delete tile at " << tile->coords_y << "x" << tile->coords_x << endl;
                }
            }
        }

        template <class Lst>
        static void clean_up_pre_render(Lst tiles, int /*pos_x*/, int /*pos_y*/)
        {
            for (auto tile : tiles) {
                //gtk_layout_move(GTK_LAYOUT(this->ui_widget()), tile->img, -1000, -1000);
            //    //gtk_widget_destroy(GTK_WIDGET(tile->img));
            //    //delete tile;
                tile = tile;
            }
            //tiles.clear();
        }
    };
};


#endif /* INCL_SCROLLING_IMAGE_H */
