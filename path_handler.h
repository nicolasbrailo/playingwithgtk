#ifndef INCL_PATH_HANDLER_H
#define INCL_PATH_HANDLER_H

#include "gtk_helper/simple_list_widget.h"
#include <string>

class Path_Handler: public Gtk_Helper::Simple_List_Widget
{
    protected:
        void element_activated(const std::string &cd);

    public:
        typedef void (*Dir_Changed_CB)(const Path_Handler*);

        Path_Handler(const std::string &curr_dir, Dir_Changed_CB cb = NULL);
        
        vector<string> get_files_on_current_dir(const vector<string> extensions) const;

    private:
       std::string curr_dir;
       Dir_Changed_CB on_dir_change;
};


#endif /* INCL_PATH_HANDLER_H */
