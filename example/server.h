#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <sys/types.h>

extern const char* program_name;

//If nonzero, print verbose message
extern int verbose;

//Like maloc, except aborts the program if allocation fails
extern void* xmalloc (size_t size);
//lIKE REALLOC, except aborts the program if allocation fails.
extern void* xrealloc (void* ptr, size_t size);
//Like strdup, except aborts then program if allocation fails.
extern char* xstrdup (const char* s);
//Print error message for a fail call operation
extern void system_error (const char* operation);
//print error message for a fail involving cause
extern void error (const char* cause , const char* message);
//print the directory containing the running program's executable
//return value is a memory buffer that the caller must deallocate using free
extern char* get_self_executable_dir();

//------------symbols defined in module.c-----------------

//an instance of a loaded server module
struct server_module {
    //The shared library handle corresponding to the loaded module
    void* handle;
    //a name describing the module
    const char* name;
    //the function that generates teh HTML results for this module
    void (* generate_function)(int);
};

//The directory from which modules are loaded
extern char* module_dir;

//attempt to load a server module with the name module_path
extern struct server_module* module_open (const char* module_path);

//----------symbols defined in server.c----------

//run server on local_address and port-----------
extern void server_run (struct in_addr local_addr, uint16_t port);


#endif
