#include <apr.h>
#include <apr_general.h>

int main(int argc, const char * const * argv, const char * const *env)
{
    apr_app_initialize(&argc, &argv, &env);


    apr_terminate();
}
