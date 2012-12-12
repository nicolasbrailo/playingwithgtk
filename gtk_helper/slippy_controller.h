#ifndef INCL_GTK_HELPER_MOUSE_DRAGGABLE_H
#define INCL_GTK_HELPER_MOUSE_DRAGGABLE_H

#include <gtk/gtk.h>
#include "general.h"

namespace Gtk_Helper {

/**
 * A wrapper to detect all UI actions related to slippy controllers:
 * mouse clicks, mouse drags and mouse wheel scrool
 */
template <int move_threshold_px>
class Slippy_Controller
{
    protected:
        // We need the widget with a protected access, so a child object
        // can use it to mask itself as a plain gtk object
        GtkWidget *widget;

    private:
        bool mouse_is_pressed;
        int drag_start_x, drag_start_y;

    public:
        Slippy_Controller(GtkWidget *draggable_widget)
                : widget(draggable_widget),
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

        GtkWidget* ui_widget(){ return this->widget; }

        virtual void mouse_dragged(int /*dx*/, int /*dy*/) {}
        virtual void mouse_clicked(int /*x*/, int /*y*/) {}
        virtual void mouse_scrolled(bool /* scroll_up */, int /*x*/, int /*y*/) {}

    private:
        bool mouse_moved(int x, int y) {
            if (not mouse_is_pressed) return GDK_SHOULD_CONTINUE_PROCESSING;

            int dx = x - drag_start_x;
            int dy = y - drag_start_y;

            if (dx < move_threshold_px and dx > -move_threshold_px) return GDK_SHOULD_CONTINUE_PROCESSING;
            if (dy < move_threshold_px and dy > -move_threshold_px) return GDK_SHOULD_CONTINUE_PROCESSING;

            drag_start_x = x;
            drag_start_y = y;

            mouse_dragged(dx, dy);

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
                    mouse_clicked(x, y);
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
            { self->mouse_scrolled((evt->direction == GDK_SCROLL_UP), evt->x, evt->y); }
};


} /* namespace Gtk_Helper */

#endif /* INCL_GTK_HELPER_MOUSE_DRAGGABLE_H */
