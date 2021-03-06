#include "path_handler.h"

#include <errno.h>
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
static vector<string> get_files(const string &path, const vector<string> extensions);
static vector<string> get_subdirs(const string &path);


Path_Handler::Path_Handler(const string &curr_dir, Dir_Changed_CB *cb)
        : curr_dir(curr_dir), callback(cb)
{
   this->load_list(get_subdirs(curr_dir));
}

void Path_Handler::element_activated(const string &cd)
{
    auto new_dir = change_dir(this->curr_dir, cd);
    this->curr_dir = new_dir;
    this->load_list(get_subdirs(new_dir));

    if (this->callback)
    {
        // TODO: looks ugly
        this->callback->on_dir_changed(this);
    }
}

vector<string> Path_Handler::get_files_on_current_dir(const vector<string> extensions) const
{
    return get_files(this->curr_dir, extensions);
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
        cerr << "Error reading " << path << ": ";
        switch(errno) {
            case EACCES: cout << "Permission denied."; break;
            case EBADF : cout << "fd is not a valid file descriptor opened for reading."; break;
            case EMFILE: cout << "Too many file descriptors in use by process."; break;
            case ENFILE: cout << "Too many files are currently open in the system."; break;
            case ENOENT: cout << "Directory does not exist, or name is an empty string."; break;
            case ENOMEM: cout << "Insufficient memory to complete the operation."; break;
        }
        cerr << endl;

        return;
    }

    struct dirent *ep;
    while((ep = readdir(dp)))
        callback(ep);

    closedir(dp);
}

static bool ends_with(const string &fname, const string &ext)
{
    if (fname.length() >= ext.length()) {
        return (0 == fname.compare(fname.length() - ext.length(), ext.length(), ext));
    } else {
        return false;
    }
}

static vector<string> get_files(const string &path, const vector<string> extensions)
{
    vector<string> files;

    auto f = [&files, &extensions](struct dirent *ep) {
        if (DT_REG != ep->d_type) return;

        // TODO: Refactor this out
        string fname = ep->d_name;

        for (auto i : extensions)
        {
            if (ends_with(fname, i))
            {
                files.push_back(fname);
                break;
            }
        }
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

