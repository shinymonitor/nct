#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>

#define PROJECT_NAME_MAX_LEN 16
#define PATH_MAX_LEN 256
#define CONFIG_MAX_LEN 256
#define COMMAND_MAX_LEN 512

#define VERSION "2.0"

#define ASCII_LOGO \
    "   _                                 ___\n"\
    "° /_) °         ___ ___     _____   (   )\n"\
    " // . ___ _    (   __  \\   /  _  \\   | |__\n"\
    "|| °/. ° |\\\\    | /  \\ |  |  / \\ |  (   __)\n"\
    "\\ \\/ .o  / \\|   | |  | |  |  | |_|   | | ___\n"\
    " \\  \\   /   \\   | |  | |  |  |  _    | |(   )\n"\
    " \\\\___\\/     |  | |  | |  |  | | |   | | | |\n"\
    "  \\          /  | |  | |  |  \\_/ |   \\ \\_/ /\n"\
    "   \\        /  (___)(___)  \\_____/    \\___/\n"\
    "    \\____ /\n"

#define HELP_MESSAGE \
    "Usage: nct [COMMAND] [ARGS]\n\n"\
    "Commands:\n"\
    "\tinit    [NAME] Initializes a project with a given name and make .nct/.nct\n"\
    "\tbuild   [ARGS] Builds project using build.c\n"\
    "\ttest    [ARGS] Build and run test.c\n"\
    "\tversion Print logo ascii art and version number\n"\
    "\thelp    Print this message\n"

#define CONFIG_FILE_CONTENT \
    "test_compile=gcc test.c -o .nct/test -Wall -Wextra\n"\
    "test_binary=.nct/test\n"\
    "build_compile=gcc build.c -o .nct/build -Wall -Wextra\n"\
    "build_binary=.nct/build\n"

#define MAIN_C_CONTENT \
    "#include <stdio.h>\n"\
    "int main(){\n"\
    "    printf(\"Hello World\\n\");\n"\
    "    return 0;\n"\
    "}\n"

#define BUILD_H_CONTENT \
    "#include <stdlib.h>\n"\
    "#include <stdio.h>\n"\
    "#include <stdbool.h>\n"\
    "#include <sys/stat.h>\n"\
    "#include <time.h>\n"\
    "#include <string.h>\n"\
    "//============================================\n"\
    "bool argument_is(int i, char* argument, int argc, char** argv);\n"\
    "bool file_exists(char* file_name);\n"\
    "bool rm_file(char* file_name);\n"\
    "bool rm_dir(char* dir_name);\n"\
    "bool copy_file(char* src, char* dest);\n"\
    "bool is_newer(char* file1, char* file2);\n"\
    "bool compile_if_changed(char* src, char* bin, char* command);\n"\
    "bool run_command(char* command);\n"\
    "bool fetch_to_lib(char* file_name, char* url);\n"\
    "bool fetch_to_lib_if_missing(char* file_name, char* url);\n"\
    "void print_info(char* message);\n"\
    "void print_error(char* message);\n"\
    "//============================================\n"\
    "#ifdef _WIN32\n"\
    "\t#include <direct.h>\n"\
    "\t#define MKDIR(path) _mkdir(path)\n"\
    "\t#define RM_CMD \"del /Q %%s\"\n"\
    "\t#define RMDIR_CMD \"rmdir /S /Q %%s\"\n"\
    "\t#define CP_CMD \"copy /Y %%s %%s\"\n"\
    "\t#define FETCH_CMD \"curl -L -o lib\\\\%%s %%s\"\n"\
    "#else\n"\
    "\t#define MKDIR(path) mkdir(path, 0755)\n"\
    "\t#define RM_CMD \"rm %%s\"\n"\
    "\t#define RMDIR_CMD \"rm -rf %%s\"\n"\
    "\t#define CP_CMD \"cp %%s %%s\"\n"\
    "\t#define FETCH_CMD \"wget -q --show-progress %%s -O lib/%%s\"\n"\
    "#endif\n"\
    "//============================================\n"\
    "#define COMMAND_MAX_LEN 512\n"\
    "bool argument_is(int i, char* argument, int argc, char** argv){\n"\
	"\tif (argc<i+1) return false;\n"\
	"\treturn (strcmp(argv[i], argument)==0);\n"\
    "}\n"\
    "bool file_exists(char* file_name){\n"\
    "\tFILE *file;\n"\
    "\tif ((file = fopen(file_name, \"r\"))) {fclose(file); return true;}\n"\
    "\treturn false;\n"\
    "}\n"\
    "bool rm_file(char* file_name){\n"\
	"\tif (!file_exists(file_name)) return false;\n"\
	"\tchar command[COMMAND_MAX_LEN];\n"\
	"\tsnprintf(command, sizeof(command), RM_CMD, file_name);\n"\
	"\treturn !system(command);\n"\
    "}\n"\
    "bool rm_dir(char* dir_name){\n"\
    "\tchar command[COMMAND_MAX_LEN];\n"\
    "\tsnprintf(command, sizeof(command), RMDIR_CMD, dir_name);\n"\
    "\treturn !system(command);\n"\
    "}\n"\
    "bool copy_file(char* src, char* dest){\n"\
    "\tchar command[COMMAND_MAX_LEN];\n"\
    "\tsnprintf(command, sizeof(command), CP_CMD, src, dest);\n"\
    "\treturn !system(command);\n"\
    "}\n"\
    "static inline time_t get_mtime(char* filename) {\n"\
    "\tstruct stat st;\n"\
    "\tif (stat(filename, &st) == -1) return 0;\n"\
    "\treturn st.st_mtime;\n"\
    "}\n"\
    "bool is_newer(char* file1, char* file2){return get_mtime(file1)>get_mtime(file2);}\n"\
    "bool run_command(char* command){return !system(command);}\n"\
    "bool compile_if_changed(char* src, char* bin, char* command){\n"\
    "\tif (!is_newer(src, bin)) return true;\n"\
    "\treturn !system(command);\n"\
    "}\n"\
    "bool fetch_to_lib(char* file_name, char* url){\n"\
    "\tchar command[COMMAND_MAX_LEN];\n"\
    "\tsnprintf(command, sizeof(command), FETCH_CMD, url, file_name);\n"\
    "\treturn !system(command);\n"\
    "}\n"\
    "bool fetch_to_lib_if_missing(char* file_name, char* url){\n"\
    "\tchar full_path[COMMAND_MAX_LEN];\n"\
    "\tsnprintf(full_path, sizeof(full_path), \"lib/%%s\", file_name);\n"\
    "\tif(file_exists(full_path)) return true;\n"\
    "\tchar command[COMMAND_MAX_LEN];\n"\
    "\tsnprintf(command, sizeof(command), FETCH_CMD, url, file_name);\n"\
    "\treturn !system(command);\n"\
    "}\n"\
    "void print_info(char* message){printf(\"[INFO] %%s\\n\", message);}\n"\
    "void print_error(char* message){printf(\"\\x1B[31;1;4m[ERROR] %%s\\n\\x1B[0m\", message);}\n"

#define BUILD_C_CONTENT \
    "#include \"build.h\"\n"\
    "#define COMPILER \"gcc\"\n"\
    "#define SRC \"src/main.c\"\n"\
    "#define BIN \"build/%s\"\n"\
    "#define FLAGS \"-Wall -Wextra -Werror -O3\"\n\n"\
    "int main(){\n"\
    "    run_command(COMPILER\" \"SRC\" -o \"BIN\" \" FLAGS);\n"\
    "    return 0;\n"\
    "}\n"

#define TEST_C_CONTENT \
    "#include <stdio.h>\n"\
    "int main(){\n"\
    "    printf(\"No tests written\\n\");\n"\
    "    return 0;\n"\
    "}\n"

static inline time_t get_mtime(char *filename) {
    struct stat st;
    if (stat(filename, &st) == -1) return 0;
    return st.st_mtime;
}
static inline bool file_exists(char *file_name){
    FILE *file;
    if ((file = fopen(file_name, "r"))) {fclose(file); return true;}
    return false;
}
#ifdef _WIN32
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
#else
    #define MKDIR(path) mkdir(path, 0755)
#endif
static inline bool mkdir_safe(const char *path) {
    if (MKDIR(path) == -1) {perror("mkdir"); return false;}
    return true;
}
static inline bool get_config(FILE* file, const char* config_name, char* config_buffer){
    char line[CONFIG_MAX_LEN * 2];
    rewind(file);
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '#') continue;
        char *equals = strchr(line, '=');
        if (!equals) continue;
        *equals = '\0';
        char *value = equals + 1;
        char *newline = strchr(value, '\n');
        if (newline) *newline = '\0';
        if (strcmp(line, config_name) == 0) {
            strncpy(config_buffer, value, CONFIG_MAX_LEN - 1);
            config_buffer[CONFIG_MAX_LEN - 1] = '\0';
            return true;
        }
    }
    return false;
}

int main(int argc, char** argv){
    if(argc==1) {printf("No command was given. Run 'nct help' for more information.\n"); return 1;}
    else if(strcmp(argv[1], "version")==0) {printf(ASCII_LOGO"\n                NCT VERSION: "VERSION"\n\n"); return 0;}
    else if(strcmp(argv[1], "help")==0) {printf(HELP_MESSAGE); return 0;}
    else if(strcmp(argv[1], "init")==0) {
        if(argc==2) {printf("No project name was given. Run 'nct help' for more information.\n"); return 1;}
        if(strlen(argv[2])>PROJECT_NAME_MAX_LEN) {printf("Project name too long (max %d chars).\n", PROJECT_NAME_MAX_LEN); return 1;}
        if (!mkdir_safe(argv[2])) return 1;
        char path[PATH_MAX_LEN];
        snprintf(path, sizeof(path), "%s/.nct", argv[2]);
        if (!mkdir_safe(path)) return 1;
        snprintf(path, sizeof(path), "%s/build", argv[2]);
        if (!mkdir_safe(path)) return 1;
        snprintf(path, sizeof(path), "%s/lib", argv[2]);
        if (!mkdir_safe(path)) return 1;
        snprintf(path, sizeof(path), "%s/src", argv[2]);
        if (!mkdir_safe(path)) return 1;
        snprintf(path, sizeof(path), "%s/.nct/.nct", argv[2]);
        FILE *config_file = fopen(path, "w");
        if (!config_file) { perror("fopen"); return 1; }
        fprintf(config_file, CONFIG_FILE_CONTENT);
        fclose(config_file);
        snprintf(path, sizeof(path), "%s/src/main.c", argv[2]);
        FILE *mainc = fopen(path, "w");
        if (!mainc) { perror("fopen main.c"); return 1; }
        fprintf(mainc, MAIN_C_CONTENT);
        fclose(mainc);
        snprintf(path, sizeof(path), "%s/build.h", argv[2]);
        FILE *buildh = fopen(path, "w");
        if (!buildh) { perror("fopen build.h"); return 1; }
        fprintf(buildh, BUILD_H_CONTENT);
        fclose(buildh);
        snprintf(path, sizeof(path), "%s/build.c", argv[2]);
        FILE *buildc = fopen(path, "w");
        if (!buildc) { perror("fopen build.c"); return 1; }
        fprintf(buildc, BUILD_C_CONTENT, argv[2]);
        fclose(buildc);
        snprintf(path, sizeof(path), "%s/test.c", argv[2]);
        FILE *testc = fopen(path, "w");
        if (!testc) { perror("fopen test.c"); return 1; }
        fprintf(testc, TEST_C_CONTENT);
        fclose(testc);
        return 0;
    }
    else if(strcmp(argv[1], "test")==0) {
        FILE* config_file = fopen(".nct/.nct", "r");
        if(!config_file) {printf("Couldn't open .nct/.nct. Make sure that a valid .nct/.nct exist or create one using \'nct init\'\n"); return 1;}
        char test_compile[CONFIG_MAX_LEN]={0};
        char test_binary[CONFIG_MAX_LEN]={0};
        if (!get_config(config_file, "test_compile", test_compile)) {
            printf("test_compile configuration not found\n");
            fclose(config_file);
            return 1;
        }
        if (!get_config(config_file, "test_binary", test_binary)) {
            printf("test_binary configuration not found\n");
            fclose(config_file);
            return 1;
        }
        fclose(config_file);
        if (!file_exists(test_binary)) {if(system(test_compile)) {printf("Couldn't run test compile command: %s\n", test_compile); return 1;}}
        else if (get_mtime("test.c")>get_mtime(test_binary)) if(system(test_compile)) {printf("Couldn't run test compile command: %s\n", test_compile); return 1;}
        char run_test_command[COMMAND_MAX_LEN];
        snprintf(run_test_command, sizeof(run_test_command), "%s", test_binary);
        for(int i=2; i<argc; ++i) {
            if (strlen(run_test_command)+strlen(argv[i])+1>COMMAND_MAX_LEN) {printf("Too many arguments and/or command too long.\n"); return 1;}
            strcat(run_test_command, " ");
            strcat(run_test_command, argv[i]);
        }
        if(system(run_test_command)) {printf("Test failed or returned non-zero exit code\n"); return 1;}
    }
    else if(strcmp(argv[1], "build")==0) {
        FILE* config_file = fopen(".nct/.nct", "r");
        if(!config_file) {printf("Couldn't open .nct/.nct. Make sure that a valid .nct/.nct exist or create one using \'nct init\'\n"); return 1;}
        char build_compile[CONFIG_MAX_LEN]={0};
        char build_binary[CONFIG_MAX_LEN]={0};
        if (!get_config(config_file, "build_compile", build_compile)) {
            printf("build_compile configuration not found\n");
            fclose(config_file);
            return 1;
        }
        if (!get_config(config_file, "build_binary", build_binary)) {
            printf("build_binary configuration not found\n");
            fclose(config_file);
            return 1;
        }
        fclose(config_file);
        if (!file_exists(build_binary)) {if(system(build_compile)) {printf("Couldn't run build compile command: %s\n", build_compile); return 1;}}
        else if (get_mtime("build.c")>get_mtime(build_binary)) if(system(build_compile)) {printf("Couldn't run build compile command: %s\n", build_compile); return 1;}
        char run_build_command[COMMAND_MAX_LEN];
        snprintf(run_build_command, sizeof(run_build_command), "%s", build_binary);
        for(int i=2; i<argc; ++i) {
            if (strlen(run_build_command)+strlen(argv[i])+1>COMMAND_MAX_LEN) {printf("Too many arguments and/or command too long.\n"); return 1;}
            strcat(run_build_command, " ");
            strcat(run_build_command, argv[i]);
        }
        if(system(run_build_command)) {printf("Build failed or returned non-zero exit code\n"); return 1;}
    }
    else {printf("Unknown command was given. Run 'nct help' for more information.\n"); return 1;}
    return 0;
}
