#ifndef INCL_GTK_HELPER_FUNCTIONS_H
#define INCL_GTK_HELPER_FUNCTIONS_H

#include <gtk/gtk.h>
#include <string>

namespace Gtk_Helper {
    using std::string;

    class ResizableContainer
    {
        ResizableContainer *autoresizable_obj;

        public:
            ResizableContainer(ResizableContainer *autoresizable_obj=NULL)
                    : autoresizable_obj(autoresizable_obj)
            {}

            void set_autoresizable_obj(ResizableContainer *p) { this->autoresizable_obj = p; }

            void resized(int width, int height)
            {
                // Resize & then propagate the resize to the child
                this->set_size(width, height);
                if (this->autoresizable_obj)
                    this->autoresizable_obj->resized(width, height);
            }

            virtual void set_size(int width, int height) = 0;
    };

    template <class Widget, typename CB>
    auto connect2(Widget wdgt, const string &event, CB callback, void* data=NULL)
        -> decltype( g_signal_connect(NULL, NULL, NULL, NULL) )
    {
        return g_signal_connect(wdgt, event.c_str(), G_CALLBACK(callback), data);
    }


    template <class Listener_Class, class Method_Ptr>
    void connect(const string &event, Listener_Class *self, Method_Ptr callback)
    {
        struct Member_Method_Wrapper {
            Listener_Class *self;
            Method_Ptr method;
            Member_Method_Wrapper(Listener_Class *self, Method_Ptr method)
                : self(self), method(method) {}
        };

        // Get the return type of the callback
        typedef decltype( (self->*callback)() ) cb_ret_t;

        // A thin lambda-callback to "cast" and resend the event received
        // to the real object connected to it
        auto f = [](GtkWidget*, GdkEvent*, void *real_cb, void*) -> cb_ret_t {
            Member_Method_Wrapper *cb = static_cast<Member_Method_Wrapper*>(real_cb);
            Listener_Class *self = cb->self;
            Method_Ptr method= cb->method;
            return (self->*method)();
        };

        // Typedef to cast lambdas into GTK-callbacks (the cast isn't automatic)
        typedef cb_ret_t (*gtk_cb_t)(GtkWidget*, GdkEvent*, void*, void*);
        // Explicitly tell the compiler we want this casted as a normal function
        // ptr, otherwise GTK will receive a lambda obect and will complain
        gtk_cb_t gtk_cb = f;

        // Cast the listener object to Gtk* so we can get a ptr directly to the
        // GTK widget: 
        GtkWidget* widget_ptr = (*self);

        // This object is needed because we can't capture the self ptr and the member
        // callback ptr:
        // 1. Lambda object is created
        // 2. We connect the signal to the lambda
        // 3. We exit this function, thus destroying the lambda object
        // 4. When the even fires, the (already dead!) lambda is executed.
        // This means once we leave the connect function the memory for self and 
        // method is destroyed, thus capturing them would mean that upon execution of
        // the lambda we'd be reading invalid memory. This also means something
        // more curious, by the time it's executed the f object would have been
        // destroyed (ie the lambda wouldn't exist anymore) so it's actually
        // executing on a dead object, which is valid as long as we don't try to
        // dereference itself. This is akin to the "delete this" pattern.
        // The consequence of this is that the real_cb pointer is lost too, so
        // the memory for this callback will never be freed. This is OK for this
        // program because we assume the events will never be disconnected; a
        // callback manager object would be needed if signals like these are
        // ever disconnected.
        auto real_cb = new Member_Method_Wrapper(self, callback);

        // Connect the event
        g_signal_connect(widget_ptr, event.c_str(), G_CALLBACK(gtk_cb), real_cb);
    }


    /**
     * Global behavior to make GTK a bit safer for people who don't RTFM
     */
    class Gtk_Object
    {
        public:
            // Disallow copy ctr. This is the safest if we're handling GTK
            // stuff, which many times has refcounted ptrs
            Gtk_Object(Gtk_Object&) = delete;

        protected:
            // Make no sense to instanciate this class on its own
            Gtk_Object(){}
    };
}

#endif /* INCL_GTK_HELPER_FUNCTIONS_H */
