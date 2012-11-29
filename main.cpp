#include <iostream>
#include <time.h>
using namespace std;


#include "gtk_helper/general.h"
#include <string>


#include "ui_images.h"
#include "gtk_helper/general.h"

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


mutex global_gtk_lock;

class DefImg
{
    const string path;
    GtkWidget *img;
    thread *t;

    public:
        DefImg(const string &path)
            : path(path)
        {
            this->img = gtk_image_new_from_file("./empty.png");
        }

        const string& get_path() const { return path; }

        template <class T>
        void update(T &cache)
        {
            Global_UI_Guard ui_guard;
            auto mem_buf = cache[path];
            this->set_from_mem(mem_buf->get_length(), mem_buf->get_buf());
            gtk_widget_show(img);
        }

        GtkWidget* get_raw_ui_ptr() { return img; }

    private:
        void set_from_mem(unsigned len, const void* buf)
        {
            auto pb_loader = gdk_pixbuf_loader_new_with_type("png", NULL);
            bool ok = gdk_pixbuf_loader_write(pb_loader, (const guchar*)buf, len, NULL);
            ok = gdk_pixbuf_loader_close(pb_loader, NULL);
            ok = ok;
            auto pb =  gdk_pixbuf_loader_get_pixbuf(pb_loader);
            gtk_image_set_from_pixbuf(GTK_IMAGE(this->img), pb);
        }
};


#include "sync_queue.h"
#include <vector>

template <class Cache> class ImgCacheProc
{
    Cache &cache;
    Sync_Queue<DefImg*> queue;
    vector<thread*> executors_lst;

    public:
    ImgCacheProc(Cache &cache, unsigned executors=10)
            : cache(cache)
    {
        for (unsigned i=0; i<executors; ++i)
            executors_lst.push_back( new thread(&ImgCacheProc<Cache>::executor, this) );
    }

    virtual ~ImgCacheProc()
    {
        for (auto ex : executors_lst)
        {
            // TODO: Signal end
            ex->join();
        }
    }

    void deferr_thumbnail(DefImg *img)
    {
        queue.push(img);
    }

    private:
    void executor()
    {
        while (true) {
            DefImg* img = queue.pop();

            // Force cache fetching, if not already present
            const string &path = img->get_path();
            auto x = cache[path];
            x = x;

            // Now load the image in the UI, done separatedly because it might
            // be non thread safe: this way we can paralelize the cache update
            // and serialize the ui showing, if it's needed; this way we give
            // img the chance to have a "UI mutex"
            img->update(cache);
        }
    }
};


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
        auto ui_img = new UI_Image(path);
        cache.deferr_thumbnail(ui_img);

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
    Magick_Thumbnail_Cache cache;
    Image_Grid< DefImg > imgs;
    ImgCacheProc<Magick_Thumbnail_Cache> def_proc;
    Path_Handler dirs;

    App()
            : cache("150"),
              def_proc(cache),
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

int main(int argc, char *argv[])
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

