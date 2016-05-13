/*************************************************************************
 > File Name: config.c
 > Author: Hac
 > Mail: hac@zju.edu.cn
 > Created Time: Thu 12 May 2016 08:55:36 AM CST
 ************************************************************************/

#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>
#include "EmbGW.h"

void Load_Conf() {
    config_t cfg;
    config_setting_t *upload, *download;
    const char *path = getenv("CONF_PATH");
    const char *pwd = getenv("PROJECT_PATH");

    config_init(&cfg);
    /* Read the configuration file. If not exists, exit */
    if (!config_read_file(&cfg, path)) {
        fprintf(stderr, "Error: %s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        exit(1);
    }
    printf("Open file '%s' --- DONE\n", path);

    /* gateway */
    {
        int id;
        const char *salt;

        if (config_lookup_int(&cfg, "gateway.id", &id))
            gateway_id = (unsigned int)id;
        else {
            fprintf(stderr, "Error: Gateway ID is missing in your config file. You may add `\"id = xxx\"` in #GATEWAY field.\n");
            exit(1);
        }

        if (config_lookup_string(&cfg, "gateway.auth.salt", &salt))
            strncpy(md5_salt, salt, sizeof(md5_salt));
        else {
            fprintf(stderr, "Error: Salt for md5 hash is missing in your config file. You may add `salt = \"something\"` in #GATEWAY field.\n");
            exit(1);
        }
    }

    /* check UPLOAD && DOWNLOAD */
    upload = config_lookup(&cfg, "gateway.upload");
    download = config_lookup(&cfg, "gateway.download");
    if (upload == NULL || download == NULL) {
        fprintf(stderr, "Error: Incomplete configuration: you have to specify both #upload and #download.\n");
        exit(1);
    }

    /* Load configuration into global variables */
    // UPLOAD
    {
        int i;
        int count = config_setting_length(upload);
        if (count > MAXSERVERNUM) {
            printf("Warning: More than %d servers are provided, while only the first %d will be used.\n", MAXSERVERNUM, MAXSERVERNUM);
            count = MAXSERVERNUM;
        }
        for (i = 0; i < count; ++i) {
            config_setting_t *server = config_setting_get_elem(upload, i);
            const char *name, *addr, *type;
            int port;

            if (!(config_setting_lookup_string(server, "name", &name)
                && config_setting_lookup_int(server, "port", &port)
                && config_setting_lookup_string(server, "addr", &addr)
                && config_setting_lookup_string(server, "type", &type))) {
                fprintf(stderr, "Error: Cannot resolve the %dth item in #UPLOAD field. Please review the config file.\n", i);
                exit(1);
            }
            strncpy(server_name[server_num], name, sizeof(server_name[i-1]));
            strncpy(server_addr[server_num], addr, sizeof(server_addr[i-1]));
            server_port[server_num] = (unsigned int)port;
            if (strcmp(type, "BIN") == 0)
                server_type[server_num] = BIN;
            else if(strcmp(type, "HTTP") == 0)
                server_type[server_num] = HTTP;
            else {
                fprintf(stderr, "Error: Cannot resolve 'type = %s' in the %dth item of #UPLOAD field.\n", type, i);
                exit(1);
            }
            server_num++;
        }
    }
    // DOWNLOAD
    {
        int i;
        int count = config_setting_length(download);
        if (count > MAXSENSORNUM) {
            printf("Warning: More than %d sensors are provided, while only the first %d will be used.\n", MAXSENSORNUM, MAXSENSORNUM);
            count = MAXSENSORNUM;
        }
        for (i = 0; i < count; ++i) {
            config_setting_t *sensor = config_setting_get_elem(download, i);
            const char *name, *device, *type;

            if (!(config_setting_lookup_string(sensor, "name", &name)
                && config_setting_lookup_string(sensor, "device", &device)
                && config_setting_lookup_string(sensor, "type", &type))) {
                fprintf(stderr, "Error: Cannot resolve the %dth item in #DOWNLOAD field. Please review the config file.\n", i);
                exit(1);
            }
            strncpy(sensor_name[sensor_num], name, sizeof(sensor_name[i-1]));
            strncpy(sensor_dev[sensor_num], device, sizeof(sensor_dev[i-1]));
            if (strcmp(type, "BT") == 0)
                sensor_type[sensor_num] = BT;
            else if(strcmp(type, "R430") == 0)
                sensor_type[sensor_num] = R430;
            else {
                fprintf(stderr, "Error: Cannot resolve 'type = %s' in the %dth item of #DOWNLOAD field.\n", type, i);
                exit(1);
            }
            sensor_num++;
        }
    }

    /* log */
    {
        const char *location;
        if (!config_lookup_string(&cfg, "gateway.log.location", &location)) {
            printf("Warning: No log file location specified. Default path: '%s/log/'\n", pwd);
            sprintf(log_location, "%s/log/", pwd);
        }
        else {
            sprintf(log_location, location, strlen(location));
        }
    }

    printf("Load configuration --- DONE\n");
}
/* int main(int argc, char *argv[]) */
/* { */
    /* Load_Conf(); */

    /* [> test <] */
    /* unsigned int i; */
    /* for (i = 0; i < server_num; i++) { */
        /* printf("%s %s %d\n", server_name[i], server_addr[i], server_type[i]); */
    /* } */
    /* for (i = 0; i < sensor_num; i++) { */
        /* printf("%s %s %d\n", sensor_name[i], sensor_dev[i], sensor_type[i]); */
    /* } */
    /* return 0; */
/* } */
