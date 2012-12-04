#ifndef INCL_GTK_HELPER_MOUSE_DRAGGABLE_H
#define INCL_GTK_HELPER_MOUSE_DRAGGABLE_H

#include <gtk/gtk.h>
#include "general.h"

namespace Gtk_Helper {

template <int move_threshold_px>
class Mouse_Draggable
{
    protected:
        // We need the widget with a protected access, so a chiled object
        // can use it to mask itself as a plain gtk object
        GtkWidget *widget;

    private:
        bool mouse_is_pressed;
        int drag_start_x, drag_start_y;

    public:
        Mouse_Draggable(GtkWidget *draggable_widget)
                : widget(draggable_widget),
                  mouse_is_pressed(false)
        {
            gtk_widget_add_events(GTK_WIDGET(widget), GDK_BUTTON_RELEASE_MASK);
            gtk_widget_add_events(GTK_WIDGET(widget), GDK_BUTTON_PRESS_MASK);
            gtk_widget_add_events(GTK_WIDGET(widget), GDK_POINTER_MOTION_MASK);

            Gtk_Helper::connect_raw(widget, "button-press-event",   &Mouse_Draggable::_clicked, this);
            Gtk_Helper::connect_raw(widget, "button-release-event", &Mouse_Draggable::_released, this);
            Gtk_Helper::connect_raw(widget, "motion_notify_event",  &Mouse_Draggable::_mouse_moved, this);
        }

        GtkWidget* ui_widget(){ return this->widget; }

        virtual void mouse_dragged(int dx, int dy) = 0;

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

        void released(int, int) {
            mouse_is_pressed = false;
        }

#define FWD_CALL(method_name) \
        static void _##method_name(void*, GdkEventButton* evt,\
                                    Mouse_Draggable *self) \
            { self->method_name(evt->x, evt->y); }

        FWD_CALL(clicked);
        FWD_CALL(released);
        FWD_CALL(mouse_moved);

#undef FWD_CALL
};


} /* namespace Gtk_Helper */

#endif /* INCL_GTK_HELPER_MOUSE_DRAGGABLE_H */