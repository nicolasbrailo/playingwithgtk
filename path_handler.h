#ifndef INCL_PATH_HANDLER_H
#define INCL_PATH_HANDLER_H

#include "gtk_helper/simple_list_widget.h"
#include <string>

class Path_Handler: public Gtk_Helper::Simple_List_Widget
{
    protected:
        void element_activated(const std::string &cd);

    public:
        struct Dir_Changed_CB {
            virtual void on_dir_changed(const Path_Handler *path) = 0;
        };

        Path_Handler(const std::string &curr_dir, Dir_Changed_CB *cb = NULL);
        
        const string& get_current_path() const { return this->curr_dir; }
        vector<string> get_files_on_current_dir(const vector<string> extensions) const;

    private:
       std::string curr_dir;
       Dir_Changed_CB *callback;
};


#endif /* INCL_PATH_HANDLER_H */
