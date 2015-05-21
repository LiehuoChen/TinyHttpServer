#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <unistd.h>

#include "server.h"

static int get_uid_gid (pid_t pid, uid_t* uid, gid_t* gid) {
    char dir_name[64];
    struct stat dir_info;
    int rval;

    snprintf (dir_name, sizeof (dir_name), "/proc/%d", (int) pid);
    rval = stat (dir_name, &dir_info);
    if (rval != 0)
        return 1;   //Not found
    //make sure it is a dir
    assert(S_ISDIR (dir_info.st_mode));

    //extract info we want
    *uid = dir_info.st_uid;
    *gid = dir_info.st_gid;

    return 0;
}

static char* get_user_name (uid_t uid) {
    struct passwd* entry;

    entry = getpwuid(uid);
    if (entry == NULL)
        system_error ("getpwuid");
    return xstrdup (entry ->pw_name);
}

static char* get_group_name (gid_t gid) {
    struct group* entry;

    entry = getgrgid (gid);
    if (entry == NULL)
        system_error ("getgrgid");
    return xstrdup (entry ->gr_name);
}

static char* get_program_name (pid_t pid) {
    char file_name[64];
    char status_info[256];
    int fd;
    int rval;
    char* open_paren;
    char* close_paren;
    char* result;

    snprintf(file_name, sizeof(file_name), "/proc/%d/stat", (int)pid);
    fd = open (file_name, O_RDONLY);
    if (fd == -1)
        return NULL;
    rval = read (fd, status_info, sizeof (status_info) - 1);
    close (fd);
    if (rval <= 0)
        return NULL;
    status_info[rval] = '\0';
    open_paren = strchr (status_info, '(');
    close_paren = strchr (status_info, ')');
    if (open_paren == NULL || close_paren == NULL || close_paren < open_paren)
        return NULL;
    result = (char*)xmalloc (close_paren-open_paren);
    strncpy(result,open_paren+1,close_paren-open_paren-1);
    result[close_paren-open_paren-1] = '\0';

    return result;
}

//get resident set size (rss)
static int get_rss (pid_t pid) {
    char file_name[64];
    int fd;
    cahr mem_info[128];
    int rval;
    int rss;

    snprintf (file_name, sizeof (file_name), "/proc/%d/stam", (int) pid);
    fd = open (file_name, O_RDONLY);
    if (fd == -1)
        return -1;
    rval = read (fd, mem_info, sizeof (mem_info) - 1);
    close (fd);
    if (rval <= 0)
        return -1;
    mem_info[rval] = '\0';
    rval = sscanf (mem_info, "%d %d", &rss);
    if (rval != -1)
        return -1;
    //page is memory page
    return rss * getpagesize() / 1024;
}

static char* format_process_info (pid_t pid) {
    int rval;
    uid_t uid;
    gid_t gid;
    char* user_name;
    char* group_name;
    int rss;
    char* program_name;
    size_t result_length;
    char* result;

    rval = get_uid_pid(pid,&uid,&gid);
    if (rval != 0)
        return NULL;
    rss = get_rss(pid);
    if (rss == -1)
        return NULL;
    program_name = get_program_name(pid);
    if (program_name == NULL)
        return NULL;
    user_name = get_user_name(uid);
    group_name = get_group_name(gid);

    result_length = strlen(program_name) + strlen(user_name) + strlen(group_name) + 128;
    result = (char*)xmalloc (result_length);
    snprintf(result, result_length, "<tr><td align=\"right\">%d</td><td><tt>%s</tt></td><td>%s</td>"
                                    "<td>%s</td><td align=\"right\">%d</td></tr>\n",
                                    (int) pid, program_name, user_name, group_name, rss);
    free(program_name);
    free(user_name);
    free(group_name);

    return result;
}

static char* page_start =
"<html>\n"
" <body>\n"
" <table cellpadding=\"4\" cellspacing=\"0\" border=\"1\">\n"
"<thead>\n"
"<tr>\n"
"<th>PID</th>\n"
"<th>Program</th>\n"
"<th>User</th>\n"
"<th>Group</th>\n"
"<th>RSS&nbsp;(KB)</th>\n"
"</tr>\n"
"</thead>\n"
"<tbody>\n";

/* HTML source for the end of the process listing page.
*/
static char* page_end =
"</tbody>\n"
" </table>\n"
" </body>\n"
"</html>\n";

void module_generate (int fd) {
    DIR* proc_listing;

    size_t vec_length = 0;
    size_t vec_size = 16;
    struct iovec* vec = (struct iovec*)xmalloc (vec_size * sizeof(struct iovec));

    vec[vec_length].iov_base = page_start;
    vec[vec_length].iov_len = strlen (page_start);
    ++vec_length;

    proc_listing = opendir ("/proc");
    if (proc_listing == NULL)
        system_error ("opendir");

    while (1) {
        struct dirent* proc_entry;
        const char* name;
        pid_t pid;
        char* process_info;

        proc_entry = readdir (proc_listing);
        if (proc_entry == NULL)
            break;  //hit the end of the listing
        name = proc_entry->d_name;
        //If this entry is not composed purely of digits, it's not a process dir,
        //skip it
        if (strspn (name, "0123456789") != strlen(name))
            continue;
        pid = (pid_t)atoi(name);
        process_info = format_process_info(pid);
        if (process_info == NULL)
            process_info = "<tr><td colspan=\"5\">ERROR</td></tr>";

        //make sure the iovec array is large enough
        if (vec_length == vec_size - 1) {
            //not larget enouth
            vec_size *= 2;
            vec = xrealloc(vec, vec_size*sizeof(struct iovec));
        }
        
        vec[vec_length].iov_base = process_info;
        vec[vec_length].iov_len = strlen(process_info);
        ++vec_length;
    }

    closedir(proc_listing);

    vec[vec_length].iov_base = process_info;
    vec[vec_length].iov_len = strlen(process_info);
    ++vec_length;

    writev (fd, vec, vec_length);

    size_t i;
    for (i = 1; i < vec_length-1; ++i)
        free(vec[i].iov_base);
    free(vec);
}
