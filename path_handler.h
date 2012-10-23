#ifndef INCL_PATH_HANDLER_H
#define INCL_PATH_HANDLER_H

#include "gtk_helper/simple_list_widget.h"
#include <string>

class Path_Handler: public Gtk_Helper::Simple_List_Widget
{
    protected:
        void element_activated(const std::string &cd);

    public:
       Path_Handler(const std::string &curr_dir);

    private:
       std::string curr_dir;
};


#endif /* INCL_PATH_HANDLER_H */
