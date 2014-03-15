/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <process.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

typedef char bool;
#define false 0
#define true (!false)

bool silent = false;
bool shared = false;
bool export_all = false;
enum mode_t { mCompile, mLink, mInstall };
enum output_type_t { otGeneral, otObject, otProgram, otStaticLibrary, otDynamicLibrary };

#ifdef __EMX__
#  define SHELL_CMD  "sh"
#  define CC         "gcc"
#  define GEN_EXPORTS "emxexp"
#  define DEF2IMPLIB_CMD "emximp"
#  define SHARE_SW   "-Zdll -Zmtd"
#  define USE_OMF true
#  define TRUNCATE_DLL_NAME
#  define DYNAMIC_LIB_EXT "dll"
#  define EXE_EXT ".exe"

#  if USE_OMF
     /* OMF is the native format under OS/2 */
#    define STATIC_LIB_EXT "lib"
#    define OBJECT_EXT     "obj"
#    define LIBRARIAN      "emxomfar"
#  else
     /* but the alternative, a.out, can fork() which is sometimes necessary */
#    define STATIC_LIB_EXT "a"
#    define OBJECT_EXT     "o"
#    define LIBRARIAN      "ar"
#  endif
#endif


typedef struct {
    char *arglist[1024];
    int num_args;
    enum mode_t mode;
    enum output_type_t output_type;
    char *output_name;
    char *stub_name;
    char *tmp_dirs[1024];
    int num_tmp_dirs;
    char *obj_files[1024];
    int num_obj_files;
} cmd_data_t;

void parse_args(int argc, char *argv[], cmd_data_t *cmd_data);
bool parse_long_opt(char *arg, cmd_data_t *cmd_data);
int parse_short_opt(char *arg, cmd_data_t *cmd_data);
bool parse_input_file_name(char *arg, cmd_data_t *cmd_data);
bool parse_output_file_name(char *arg, cmd_data_t *cmd_data);
void post_parse_fixup(cmd_data_t *cmd_data);
bool explode_static_lib(char *lib, cmd_data_t *cmd_data);
int execute_command(cmd_data_t *cmd_data);
char *shell_esc(const char *str);
void cleanup_tmp_dirs(cmd_data_t *cmd_data);
void generate_def_file(cmd_data_t *cmd_data);
char *nameof(char *fullpath);
char *truncate_dll_name(char *path);


int main(int argc, char *argv[])
{
    int rc;
    cmd_data_t cmd_data;

    memset(&cmd_data, 0, sizeof(cmd_data));
    cmd_data.mode = mCompile;
    cmd_data.output_type = otGeneral;

    parse_args(argc, argv, &cmd_data);
    rc = execute_command(&cmd_data);

    if (rc == 0 && cmd_data.stub_name) {
        fopen(cmd_data.stub_name, "w");
    }

    cleanup_tmp_dirs(&cmd_data);
    return rc;
}



void parse_args(int argc, char *argv[], cmd_data_t *cmd_data)
{
    int a;
    char *arg;
    bool argused;

    for (a=1; a < argc; a++) {
        arg = argv[a];
        argused = false;

        if (arg[0] == '-') {
            if (arg[1] == '-') {
                argused = parse_long_opt(arg + 2, cmd_data);
            } else if (arg[1] == 'o' && a+1 < argc) {
                cmd_data->arglist[cmd_data->num_args++] = arg;
                arg = argv[++a];
                argused = parse_output_file_name(arg, cmd_data);
            } else {
                int num_used = parse_short_opt(arg + 1, cmd_data);
                argused = num_used > 0;

                if (num_used > 1) {
                    a += num_used - 1;
                }
            }
        } else {
            argused = parse_input_file_name(arg, cmd_data);
        }

        if (!argused) {
            cmd_data->arglist[cmd_data->num_args++] = arg;
        }
    }

    post_parse_fixup(cmd_data);
}



bool parse_long_opt(char *arg, cmd_data_t *cmd_data)
{
    char *equal_pos = strchr(arg, '=');
    char var[50];
    char value[500];

    if (equal_pos) {
        strncpy(var, arg, equal_pos - arg);
        var[equal_pos - arg] = 0;
        strcpy(value, equal_pos + 1);
    } else {
        strcpy(var, arg);
    }

    if (strcmp(var, "silent") == 0) {
        silent = true;
    } else if (strcmp(var, "mode") == 0) {
        if (strcmp(value, "compile") == 0) {
            cmd_data->mode = mCompile;
            cmd_data->output_type = otObject;
        }

        if (strcmp(value, "link") == 0) {
            cmd_data->mode = mLink;
        }

        if (strcmp(value, "install") == 0) {
            cmd_data->mode = mInstall;
        }
    } else if (strcmp(var, "shared") == 0) {
        shared = true;
    } else if (strcmp(var, "export-all") == 0) {
        export_all = true;
    } else {
        return false;
    }

    return true;
}



int parse_short_opt(char *arg, cmd_data_t *cmd_data)
{
    if (strcmp(arg, "export-dynamic") == 0) {
        return 1;
    }

    if (strcmp(arg, "module") == 0) {
        return 1;
    }

    if (strcmp(arg, "Zexe") == 0) {
        return 1;
    }

    if (strcmp(arg, "avoid-version") == 0) {
        return 1;
    }

    if (strcmp(arg, "prefer-pic") == 0) {
        return 1;
    }

    if (strcmp(arg, "prefer-non-pic") == 0) {
        return 1;
    }

    if (strcmp(arg, "version-info") == 0 ) {
        return 2;
    }

    if (strcmp(arg, "no-install") == 0) {
        return 1;
    }

    return 0;
}



bool parse_input_file_name(char *arg, cmd_data_t *cmd_data)
{
    char *ext = strrchr(arg, '.');
    char *name = strrchr(arg, '/');
    char *newarg;

    if (!ext) {
        return false;
    }

    ext++;

    if (name == NULL) {
        name = strrchr(arg, '\\');

        if (name == NULL) {
            name = arg;
        } else {
            name++;
        }
    } else {
        name++;
    }

    if (strcmp(ext, "lo") == 0) {
        newarg = (char *)malloc(strlen(arg) + 10);
        strcpy(newarg, arg);
        strcpy(newarg + (ext - arg), OBJECT_EXT);
        cmd_data->arglist[cmd_data->num_args++] = newarg;
        cmd_data->obj_files[cmd_data->num_obj_files++] = newarg;
        return true;
    }

    if (strcmp(ext, "la") == 0) {
        newarg = (char *)malloc(strlen(arg) + 10);
        strcpy(newarg, arg);
        newarg[pathlen] = 0;
        strcat(newarg, ".libs/");

        if (strncmp(name, "lib", 3) == 0) {
            name += 3;
        }

        strcat(newarg, name);
        ext = strrchr(newarg, '.') + 1;

        if (shared && cmd_data->mode == mInstall) {
          strcpy(ext, DYNAMIC_LIB_EXT);
          newarg = truncate_dll_name(newarg);
        } else {
          strcpy(ext, STATIC_LIB_EXT);
        }

        cmd_data->arglist[cmd_data->num_args++] = newarg;
        return true;
    }

    if (strcmp(ext, "c") == 0) {
        if (cmd_data->stub_name == NULL) {
            cmd_data->stub_name = (char *)malloc(strlen(arg) + 4);
            strcpy(cmd_data->stub_name, arg);
            strcpy(strrchr(cmd_data->stub_name, '.') + 1, "lo");
        }
    }

    if (strcmp(name, CC) == 0 || strcmp(name, CC EXE_EXT) == 0) {
        if (cmd_data->output_type == otGeneral) {
            cmd_data->output_type = otObject;
        }
    }

    return false;
}



bool parse_output_file_name(char *arg, cmd_data_t *cmd_data)
{
    char *name = strrchr(arg, '/');
    char *ext = strrchr(arg, '.');
    char *newarg = NULL, *newext;

    if (name == NULL) {
        name = strrchr(arg, '\\');

        if (name == NULL) {
            name = arg;
        } else {
            name++;
        }
    } else {
        name++;
    }

    if (!ext) {
        cmd_data->stub_name = arg;
        cmd_data->output_type = otProgram;
        newarg = (char *)malloc(strlen(arg) + 5);
        strcpy(newarg, arg);
        strcat(newarg, EXE_EXT);
        cmd_data->arglist[cmd_data->num_args++] = newarg;
        cmd_data->output_name = newarg;
        return true;
    }

    ext++;

    if (strcmp(ext, "la") == 0) {
        cmd_data->stub_name = arg;
        cmd_data->output_type = shared ? otDynamicLibrary : otStaticLibrary;
        newarg = (char *)malloc(strlen(arg) + 10);
        mkdir(".libs", 0);
        strcpy(newarg, ".libs/");

        if (strncmp(arg, "lib", 3) == 0) {
            arg += 3;
        }

        strcat(newarg, arg);
        newext = strrchr(newarg, '.') + 1;
        strcpy(newext, shared ? DYNAMIC_LIB_EXT : STATIC_LIB_EXT);

#ifdef TRUNCATE_DLL_NAME
        if (shared) {
          newarg = truncate_dll_name(newarg);
        }
#endif

        cmd_data->arglist[cmd_data->num_args++] = newarg;
        cmd_data->output_name = newarg;
        return true;
    }

    if (strcmp(ext, "lo") == 0) {
        cmd_data->stub_name = arg;
        cmd_data->output_type = otObject;
        newarg = (char *)malloc(strlen(arg) + 2);
        strcpy(newarg, arg);
        ext = strrchr(newarg, '.') + 1;
        strcpy(ext, OBJECT_EXT);
        cmd_data->arglist[cmd_data->num_args++] = newarg;
        cmd_data->output_name = newarg;
        return true;
    }

    return false;
}



void post_parse_fixup(cmd_data_t *cmd_data)
{
    int a;
    char *arg;
    char *ext;

    if (cmd_data->output_type == otStaticLibrary && cmd_data->mode == mLink) {
        /* We do a real hatchet job on the args when making a static library
         * removing all compiler switches & any other cruft that ar won't like
         * We also need to explode any libraries listed
         */

        for (a=0; a < cmd_data->num_args; a++) {
            arg = cmd_data->arglist[a];

            if (arg) {
                ext = strrchr(arg, '.');

                if (ext) {
                    ext++;
                }

                if (arg[0] == '-') {
                    cmd_data->arglist[a] = NULL;

                    if (strcmp(arg, "-rpath") == 0 && a+1 < cmd_data->num_args) {
                        cmd_data->arglist[a+1] = NULL;
                    }

                    if (strcmp(arg, "-R") == 0 && a+1 < cmd_data->num_args) {
                        cmd_data->arglist[a+1] = NULL;
                    }

                    if (strcmp(arg, "-version-info") == 0 && a+1 < cmd_data->num_args) {
                        cmd_data->arglist[a+1] = NULL;
                    }

                    if (strcmp(arg, "-Zstack") == 0 && a+1 < cmd_data->num_args) {
                        cmd_data->arglist[a+1] = NULL;
                    }

                    if (strcmp(arg, "-o") == 0) {
                        a++;
                    }
                }

                if (strcmp(arg, CC) == 0 || strcmp(arg, CC EXE_EXT) == 0) {
                    cmd_data->arglist[a] = LIBRARIAN " cr";
                }

                if (ext) {
                    if (strcmp(ext, "h") == 0 || strcmp(ext, "c") == 0) {
                        /* ignore source files, they don't belong in a library */
                        cmd_data->arglist[a] = NULL;
                    }

                    if (strcmp(ext, STATIC_LIB_EXT) == 0) {
                        cmd_data->arglist[a] = NULL;
                        explode_static_lib(arg, cmd_data);
                    }
                }
            }
        }
    }

    if (cmd_data->output_type == otDynamicLibrary) {
        for (a=0; a < cmd_data->num_args; a++) {
            arg = cmd_data->arglist[a];

            if (arg) {
                if (strcmp(arg, "-rpath") == 0 && a+1 < cmd_data->num_args) {
                    cmd_data->arglist[a] = NULL;
                    cmd_data->arglist[a+1] = NULL;
                }
            }
        }

        if (export_all) {
            generate_def_file(cmd_data);
        }
    }

#if USE_OMF
    if (cmd_data->output_type == otObject ||
        cmd_data->output_type == otProgram ||
        cmd_data->output_type == otDynamicLibrary) {
        cmd_data->arglist[cmd_data->num_args++] = "-Zomf";
    }
#endif

    if (shared && (cmd_data->output_type == otObject || cmd_data->output_type == otDynamicLibrary)) {
        cmd_data->arglist[cmd_data->num_args++] = SHARE_SW;
    }
}



int execute_command(cmd_data_t *cmd_data)
{
    int target = 0;
    char *command;
    int a, total_len = 0;
    char *args[4];

    for (a=0; a < cmd_data->num_args; a++) {
        if (cmd_data->arglist[a]) {
            total_len += strlen(cmd_data->arglist[a]) + 1;
        }
    }

    command = (char *)malloc( total_len );
    command[0] = 0;

    for (a=0; a < cmd_data->num_args; a++) {
        if (cmd_data->arglist[a]) {
            strcat(command, cmd_data->arglist[a]);
            strcat(command, " ");
        }
    }

    command[strlen(command)-1] = 0;

    if (!silent) {
        puts(command);
    }

    cmd_data->num_args = target;
    cmd_data->arglist[cmd_data->num_args] = NULL;
    command = shell_esc(command);

    args[0] = SHELL_CMD;
    args[1] = "-c";
    args[2] = command;
    args[3] = NULL;
    return spawnvp(P_WAIT, args[0], args);
}



char *shell_esc(const char *str)
{
    char *cmd;
    unsigned char *d;
    const unsigned char *s;

    cmd = (char *)malloc(2 * strlen(str) + 1);
    d = (unsigned char *)cmd;
    s = (const unsigned char *)str;

    for (; *s; ++s) {
        if (*s == '"' || *s == '\\') {
            *d++ = '\\';
        }
        *d++ = *s;
    }

    *d = '\0';
    return cmd;
}



bool explode_static_lib(char *lib, cmd_data_t *cmd_data)
{
    char tmpdir[1024];
    char savewd[1024];
    char cmd[1024];
    char *name;
    DIR *dir;
    struct dirent *entry;

    strcpy(tmpdir, lib);
    strcat(tmpdir, ".exploded");

    mkdir(tmpdir, 0);
    cmd_data->tmp_dirs[cmd_data->num_tmp_dirs++] = strdup(tmpdir);
    getcwd(savewd, sizeof(savewd));

    if (chdir(tmpdir) != 0)
        return false;

    strcpy(cmd, LIBRARIAN " x ");
    name = strrchr(lib, '/');

    if (name) {
        name++;
    } else {
        name = lib;
    }

    strcat(cmd, "../");
    strcat(cmd, name);
    system(cmd);
    chdir(savewd);
    dir = opendir(tmpdir);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            strcpy(cmd, tmpdir);
            strcat(cmd, "/");
            strcat(cmd, entry->d_name);
            cmd_data->arglist[cmd_data->num_args++] = strdup(cmd);
        }
    }

    closedir(dir);
    return true;
}



void cleanup_tmp_dir(char *dirname)
{
    DIR *dir;
    struct dirent *entry;
    char fullname[1024];

    dir = opendir(dirname);

    if (dir == NULL)
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            strcpy(fullname, dirname);
            strcat(fullname, "/");
            strcat(fullname, entry->d_name);
            remove(fullname);
        }
    }

    rmdir(dirname);
}



void cleanup_tmp_dirs(cmd_data_t *cmd_data)
{
    int d;

    for (d=0; d < cmd_data->num_tmp_dirs; d++) {
        cleanup_tmp_dir(cmd_data->tmp_dirs[d]);
    }
}



void generate_def_file(cmd_data_t *cmd_data)
{
    char def_file[1024];
    char implib_file[1024];
    char *ext;
    FILE *hDef;
    char *export_args[1024];
    int num_export_args = 0;
    char *cmd;
    int cmd_size = 0;
    int a;

    if (cmd_data->output_name) {
        strcpy(def_file, cmd_data->output_name);
        strcat(def_file, ".def");
        hDef = fopen(def_file, "w");

        if (hDef != NULL) {
            fprintf(hDef, "LIBRARY '%s' INITINSTANCE\n", nameof(cmd_data->output_name));
            fprintf(hDef, "DATA NONSHARED\n");
            fprintf(hDef, "EXPORTS\n");
            fclose(hDef);

            for (a=0; a < cmd_data->num_obj_files; a++) {
                cmd_size += strlen(cmd_data->obj_files[a]) + 1;
            }

            cmd_size += strlen(GEN_EXPORTS) + strlen(def_file) + 3;
            cmd = (char *)malloc(cmd_size);
            strcpy(cmd, GEN_EXPORTS);

            for (a=0; a < cmd_data->num_obj_files; a++) {
                strcat(cmd, " ");
                strcat(cmd, cmd_data->obj_files[a] );
            }

            strcat(cmd, ">>");
            strcat(cmd, def_file);
            puts(cmd);
            export_args[num_export_args++] = SHELL_CMD;
            export_args[num_export_args++] = "-c";
            export_args[num_export_args++] = cmd;
            export_args[num_export_args++] = NULL;
            spawnvp(P_WAIT, export_args[0], export_args);
            cmd_data->arglist[cmd_data->num_args++] = strdup(def_file);

            /* Now make an import library for the dll */
            num_export_args = 0;
            export_args[num_export_args++] = DEF2IMPLIB_CMD;
            export_args[num_export_args++] = "-o";

            strcpy(implib_file, ".libs/");
            strcat(implib_file, cmd_data->stub_name);
            ext = strrchr(implib_file, '.');

            if (ext)
                *ext = 0;

            strcat(implib_file, ".");
            strcat(implib_file, STATIC_LIB_EXT);

            export_args[num_export_args++] = implib_file;
            export_args[num_export_args++] = def_file;
            export_args[num_export_args++] = NULL;
            spawnvp(P_WAIT, export_args[0], export_args);
        }
    }
}



/* returns just a file's name without path or extension */
char *nameof(char *fullpath)
{
    char buffer[1024];
    char *ext;
    char *name = strrchr(fullpath, '/');

    if (name == NULL) {
        name = strrchr(fullpath, '\\');
    }

    if (name == NULL) {
        name = fullpath;
    } else {
        name++;
    }

    strcpy(buffer, name);
    ext = strrchr(buffer, '.');

    if (ext) {
        *ext = 0;
        return strdup(buffer);
    }

    return name;
}



char *truncate_dll_name(char *path)
{
    /* Cut DLL name down to 8 characters after removing any mod_ prefix */
    char *tmppath = strdup(path);
    char *newname = strrchr(tmppath, '/') + 1;
    char *ext = strrchr(tmppath, '.');
    int len;

    if (ext == NULL)
        return tmppath;

    len = ext - newname;

    if (strncmp(newname, "mod_", 4) == 0) {
        strcpy(newname, newname + 4);
        len -= 4;
    }

    if (len > 8) {
        strcpy(newname + 8, strchr(newname, '.'));
    }

    return tmppath;
}
