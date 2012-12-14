#ifndef INCL_MAP_TILE_GENERATOR_H
#define INCL_MAP_TILE_GENERATOR_H

// TODO: Investigate other map providers @ http://www.netmagazine.com/features/top-seven-alternatives-google-maps-api
struct Map_Tile_Generator
{
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



    typedef Open_Street_Map Map;

    /**
     * "cache" object to use with the Deferred_Image_Loader: must generate
     * a path to a tile from a set of coordinates
     */
    struct Deferred_Tile_Fetcher
    {
        // TODO: Return a ref instead. Is it of any use keeping a local ref?
        const std::string operator[] (const Map_Tile *img) const
        {
            auto fname = Map::get_tile_fname(img->zoom, img->coords_x, img->coords_y);

            ifstream cached_file(fname);
            if (cached_file.good())
            {
                cout << "Got from cache " << fname << endl;
                return fname;
            }

            auto url = Map::get_tile_url(img->zoom, img->coords_x, img->coords_y);
            cout << "Getting " << url << " into " << fname << endl;
            wget(url, fname);
            return fname;
        }
    } deferred_tile_fetcher;

    typedef Deferred_Image_Loader<Deferred_Tile_Fetcher, Map_Tile> Deferred_Tile_Updater;
    Deferred_Tile_Updater *deferred_tile_loader;

    int zoom_level;
    int map_offset_x, map_offset_y;
    

    ~Map_Tile_Generator()
    {
        delete deferred_tile_loader;
    }

    Map_Tile_Generator()
        : deferred_tile_loader(new Deferred_Tile_Updater(deferred_tile_fetcher, 10)),
          zoom_level(7),
          map_offset_x(64), map_offset_y(41)
    {
    }

    unsigned get_tile_width() const { return 256; }
    unsigned get_tile_height() const { return 256; }

    void zoom_out(double click_coords_x, double click_coords_y)
    {
        // http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
        // Since the next zoom level will have count*2 tiles, there will be
        // 4 subtiles for each tile if zoom in one level; this means also
        // that the upper left tile in the next zoom level will be 2*coords
        zoom_level -= 1;
        map_offset_x = (int)((click_coords_x + map_offset_x) / 2) - 1;
        map_offset_y = (int)((click_coords_y + map_offset_y) / 2) - 1;
    }

    void zoom_in(double click_coords_x, double click_coords_y)
    {
        // http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
        // Since the next zoom level will have count*2 tiles, there will be
        // 4 subtiles for each tile if zoom in one level; this means also
        // that the upper left tile in the next zoom level will be 2*coords
        zoom_level += 1;
        map_offset_x = (int)(2 * (click_coords_x + map_offset_x)) - 1;
        map_offset_y = (int)(2 * (click_coords_y + map_offset_y)) - 1;
    }

    Map_Tile* generate_tile(int coords_x, int coords_y)
    {
        int tile_x = map_offset_x + coords_x;
        int tile_y = map_offset_y + coords_y;
        auto img = new Map_Tile(tile_x, tile_y, zoom_level);
        deferred_tile_loader->process(img);
        return img;
    }

    void cleanup_all_stuff()
    {
        cout << "Cleanup deferred queue" << endl;
        // Deleting deferred_tile_loader will purge the update queue
        delete deferred_tile_loader;
        deferred_tile_loader = new Deferred_Tile_Updater(deferred_tile_fetcher, 10);
    }


    void destroy(Map_Tile *tile)
    {
        // We can't directly delete this tile: since it's a deferred image ui
        // control, we might delete it before the img tile has had the chance
        // to finish updating. If this happens, we'll delete the object, then
        // the tile fetcher will be done and it'll try to update the (now
        // deleted) ui object. Nasal demons ftw. Instead we'll tell it to
        // delete itself once the update was completed
        tile->be_gone();
    }


    static void map_tile_pos_to_coords(double tile_x, double tile_y,
                                       double *coord_x, double *coord_y)
    {
        // http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
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


    typedef Map_Tile UI_Tile_Image;
};

#endif /* INCL_MAP_TILE_GENERATOR_H */
