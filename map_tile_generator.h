#ifndef INCL_MAP_TILE_GENERATOR_H
#define INCL_MAP_TILE_GENERATOR_H

#include <string>

// Fwd decl
template <class Cache, class UI_Image> class Deferred_Image_Loader;
struct Deferred_Tile_Fetcher;

#include "image.h"
struct Map_Tile : public Image_From_File 
{
    int coords_x, coords_y, zoom;

    Map_Tile(int coords_x, int coords_y, int zoom)
        : Image_From_File("./map_loading.gif"),
          coords_x(coords_x), coords_y(coords_y), zoom(zoom)
    {
    }

    Map_Tile* get_path() { return this; }
};


struct Map_Tile_Generator
{
    // More fwd decls
    typedef Deferred_Image_Loader<Deferred_Tile_Fetcher,
                                  Map_Tile> Deferred_Tile_Updater;
    Deferred_Tile_Updater *deferred_tile_loader;

    int zoom_level;
    int map_offset_x, map_offset_y;
    

    ~Map_Tile_Generator();
    Map_Tile_Generator();

    unsigned get_tile_width() const { return 256; }
    unsigned get_tile_height() const { return 256; }

    void zoom_in(double click_coords_x, double click_coords_y);
    void zoom_out(double click_coords_x, double click_coords_y);

    Map_Tile* generate_tile(int coords_x, int coords_y);

    void cleanup_all_stuff();


    void destroy(Map_Tile *tile);


    void map_tile_pos_to_coords(double tile_x, double tile_y,
                                       double *coord_x, double *coord_y);

    void map_coords_to_tile(double coord_x, double coord_y,
                                       double *tile_x, double *tile_y);

    typedef Map_Tile UI_Tile_Image;
};

#endif /* INCL_MAP_TILE_GENERATOR_H */
