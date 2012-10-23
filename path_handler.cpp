#include "path_handler.h"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::cerr;
using std::endl;

// Some helper functions
static string change_dir(const string &start_path, const string &cd);
static vector<string> get_files(const string &path);
static vector<string> get_subdirs(const string &path);


Path_Handler::Path_Handler(const string &curr_dir)
        : curr_dir(curr_dir)
{
   this->load_list(get_subdirs(curr_dir));
}

void Path_Handler::element_activated(const string &cd)
{
    auto new_dir = change_dir(this->curr_dir, cd);
    this->curr_dir = new_dir;
    this->load_list(get_subdirs(new_dir));
}


static string change_dir(const string &start_path, const string &cd)
{
    const unsigned last = start_path.length();

    if (cd == "..")
    {
        const unsigned up = start_path.rfind('/', last-2);
        return start_path.substr(0, up);
    }

    if (start_path[last] == '/')
        return start_path + cd;
    else
        return start_path + '/' + cd;
}

template <class CB>
static void _read_dir_impl(const string &path, CB callback)
{
    auto dp = opendir(path.c_str());
    if (dp == NULL)
    {
        cerr << "Error reading " << path << endl;
        return;
    }

    struct dirent *ep;
    while((ep = readdir(dp)))
        callback(ep);

    closedir(dp);
}

static vector<string> get_files(const string &path)
{
    vector<string> files({".."});

    auto f = [&files](struct dirent *ep) {
        if (DT_REG != ep->d_type) return;
        files.push_back(ep->d_name);
    };

    _read_dir_impl(path, f);
    return files;
}

static vector<string> get_subdirs(const string &path)
{
    vector<string> subdirs({".."});

    auto f = [&subdirs](struct dirent *ep) {
        if (DT_DIR != ep->d_type) return;
        // Skip hidden files and ., ..
        if ('.' == ep->d_name[0]) return;
        subdirs.push_back(ep->d_name);
    };

    _read_dir_impl(path, f);
    return subdirs;
}

