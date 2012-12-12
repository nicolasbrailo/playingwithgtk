#ifndef INCL_GTK_HELPER_MOUSE_DRAGGABLE_H
#define INCL_GTK_HELPER_MOUSE_DRAGGABLE_H

#include <gtk/gtk.h>
#include "general.h"

namespace Gtk_Helper {

struct Slippy_Events_Callbacks
{
    virtual void mouse_dragged(int /*dx*/, int /*dy*/) = 0;
    virtual void mouse_clicked(int /*x*/, int /*y*/) = 0;
    virtual void mouse_scrolled(bool /* scroll_up */, int /*x*/, int /*y*/) = 0;
};

/**
 * A wrapper to detect all UI actions related to slippy controllers:
 * mouse clicks, mouse drags and mouse wheel scrool
 */
template <int move_threshold_px>
class Slippy_Controller
{
    protected:
        Slippy_Events_Callbacks *callback;

    private:
        bool mouse_is_pressed;
        int drag_start_x, drag_start_y;

    public:
        Slippy_Controller(GtkWidget *widget, Slippy_Events_Callbacks *callback)
                : callback(callback),
                  mouse_is_pressed(false)
        {
            gtk_widget_add_events(GTK_WIDGET(widget), GDK_BUTTON_RELEASE_MASK);
            gtk_widget_add_events(GTK_WIDGET(widget), GDK_BUTTON_PRESS_MASK);
            gtk_widget_add_events(GTK_WIDGET(widget), GDK_POINTER_MOTION_MASK);

            Gtk_Helper::connect_raw(widget, "button-press-event",   &Slippy_Controller::_clicked, this);
            Gtk_Helper::connect_raw(widget, "button-release-event", &Slippy_Controller::_released, this);
            Gtk_Helper::connect_raw(widget, "motion_notify_event",  &Slippy_Controller::_mouse_moved, this);
            Gtk_Helper::connect_raw(widget, "scroll-event",  &Slippy_Controller::_mouse_scrolled, this);
        }

    private:
        bool mouse_moved(int x, int y) {
            if (not mouse_is_pressed) return GDK_SHOULD_CONTINUE_PROCESSING;

            int dx = x - drag_start_x;
            int dy = y - drag_start_y;

            if (dx < move_threshold_px and dx > -move_threshold_px) return GDK_SHOULD_CONTINUE_PROCESSING;
            if (dy < move_threshold_px and dy > -move_threshold_px) return GDK_SHOULD_CONTINUE_PROCESSING;

            drag_start_x = x;
            drag_start_y = y;

            callback->mouse_dragged(dx, dy);

            return GDK_SHOULD_STOP_PROCESSING;
        }

        void clicked(int x, int y) {
            mouse_is_pressed = true;
            drag_start_x = x;
            drag_start_y = y;
        }

        void released(int x, int y) {
            mouse_is_pressed = false;

            int dx = x - drag_start_x;
            int dy = y - drag_start_y;

            // If we're within the move_threshold then we'd have never fired a
            // mouse dragged event. We'll fire a mouse clicked event instead.
            if (dx < move_threshold_px and dx > -move_threshold_px)
                if (dy < move_threshold_px and dy > -move_threshold_px)
                    callback->mouse_clicked(x, y);
        }


#define FWD_CALL(method_name) \
        static void _##method_name(void*, GdkEventButton* evt,\
                                    Slippy_Controller *self) \
            { self->method_name(evt->x, evt->y); }
        FWD_CALL(clicked);
        FWD_CALL(released);
        FWD_CALL(mouse_moved);
#undef FWD_CALL

        static void _mouse_scrolled(void*, GdkEventScroll* evt,\
                                    Slippy_Controller *self) \
            { self->callback->mouse_scrolled((evt->direction == GDK_SCROLL_UP), evt->x, evt->y); }
};


/**
 * Specialization for a Slippy_Controller based on images using
 * a gtk layout
 */
template <int move_threshold_px>
class Slippy_Image : public Slippy_Events_Callbacks
{
    GtkWidget *ui_controller, *canvas;
    Gtk_Helper::Slippy_Controller<move_threshold_px> slippy_evts;

    public:
        Slippy_Image()
            : ui_controller(gtk_scrolled_window_new(NULL, NULL)),
              canvas(gtk_layout_new(NULL, NULL)),
              slippy_evts(canvas, this)
        {
            gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(ui_controller), canvas);
        }

        void set_size(unsigned width, unsigned height)
        {
            gtk_widget_set_usize(canvas, height, width);
            gtk_widget_set_usize(ui_controller, height, width);
        }

        void move_image(GtkWidget *img, int x, int y)
        {
            gtk_layout_move(GTK_LAYOUT(canvas), img, x, y);
            gtk_widget_show(img);
        }

        void place_image(GtkWidget *img, int x, int y)
        {
            gtk_layout_put(GTK_LAYOUT(canvas), img, x, y);
            gtk_widget_show(img);
        }

        operator GtkWidget* (){ return this->ui_controller; }
};



} /* namespace Gtk_Helper */

#endif /* INCL_GTK_HELPER_MOUSE_DRAGGABLE_H */
