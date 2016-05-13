/*************************************************************************
 > File Name: conf.c
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Thu 12 May 2016 10:29:22 AM CST
 ************************************************************************/

#include "stdio.h"
#include <stdlib.h>
#include <libconfig.h>

int main(int argc, char *argv[])
{
    config_t mycfg;
    const char *str;
    int port;

    config_init(&mycfg);
    if (config_read_file(&mycfg, "default.cfg") == CONFIG_FALSE) {
        /* config_error_type(&mycfg); */
        /* config_error_file(&mycfg); */
        /* config_error_line(&mycfg); */
        /* config_error_text(&mycfg); */
        config_destroy(&mycfg);
        exit(1);
    }
    if (config_lookup_string(&mycfg, "gateway.upload.[1].port", &str))
        printf("addr: %s\n", str);
    else
        fprintf(stderr, "NO addr specified in config file.\n");
    return 0;
}
