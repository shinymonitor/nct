#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>

#define PROJECT_NAME_MAX_LEN 16
#define PATH_MAX_LEN 256
#define CONFIG_MAX_LEN 256
#define COMMAND_MAX_LEN 512

#define VERSION "2.2"

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
    "#include <errno.h>\n"\
    "#include <stdio.h>\n"\
    "#include <stdbool.h>\n"\
    "#include <sys/stat.h>\n"\
    "#include <time.h>\n"\
    "#include <string.h>\n"\
    "//============================================\n"\
    "typedef struct {char** targets; size_t targets_count; char** dependencies; size_t dependencies_count; char** commands; size_t commands_count;} Maqe;\n"\
    "//============================================\n"\
    "bool argument_is(int i, char* argument, int argc, char** argv);\n"\
    "bool file_exists(char* file_path);\n"\
    "bool mkdir_if_not_exist(const char *path);\n"\
    "bool rm_file(char* file_path);\n"\
    "bool rm_dir(char* dir_path);\n"\
    "bool copy_file(char* src_path, char* dest_path);\n"\
    "bool is_newer(char* new_path, char* old_path);\n\n"\
    "bool run_command(char* command);\n"\
    "bool run_maqe(Maqe* maqes, size_t maqes_len);\n\n"\
    "bool fetch_to_lib(char* file_name, char* url);\n"\
    "bool fetch_to_lib_if_missing(char* file_name, char* url);\n\n"\
    "void print_info(char* message);\n"\
    "void print_error(char* message);\n"\
    "//============================================\n"\
    "#define MAX_MAQES 100\n"\
    "#define Maqe_Init() Maqe maqes[MAX_MAQES]={0}; size_t count=0; size_t temp=0\n\n"\
    "Maqe_Init();\n\n"\
    "#define S(...) (char*[]){__VA_ARGS__}, (sizeof((char*[]){__VA_ARGS__})/sizeof(char*))\n"\
    "#define Maqe_Rule(...) maqes[count]=(Maqe){__VA_ARGS__}; ++count\n"\
    "#define Maqe_Run() run_maqe(maqes, count)\n\n"\
    "#define Maqe_Reset() temp=sizeof(maqes); memset(maqes, 0, temp); count=0\n"\
    "//============================================\n"\
    "#ifdef _WIN32\n"\
    "\t#include <direct.h>\n"\
    "\t#define NL \"\\r\\n\"\n"\
    "\t#define PS \"\\\\\"\n"\
    "\t#define MKDIR(path) _mkdir(path)\n"\
    "\t#define RM_CMD \"del /Q %%s\"\n"\
    "\t#define RMDIR_CMD \"rmdir /S /Q %%s\"\n"\
    "\t#define CP_CMD \"copy /Y %%s %%s\"\n"\
    "\t#define FETCH_CMD \"curl -L -o lib\\\\%%s %%s\"\n"\
    "#else\n"\
    "\t#define NL \"\\n\"\n"\
    "\t#define PS \"/\"\n"\
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
    "bool file_exists(char* file_path){\n"\
    "\tFILE *file;\n"\
    "\tif ((file = fopen(file_path, \"r\"))) {fclose(file); return true;}\n"\
    "\treturn false;\n"\
    "}\n"\
    "bool mkdir_if_not_exist(const char *path){\n"\
    "\tif (MKDIR(path) != 0 || errno == EEXIST) return true;\n"\
    "\treturn false;\n"\
    "}\n"\
    "bool rm_file(char* file_path){\n"\
	"\tif (!file_exists(file_path)) return false;\n"\
	"\tchar command[COMMAND_MAX_LEN];\n"\
	"\tsnprintf(command, sizeof(command), RM_CMD, file_path);\n"\
	"\treturn !system(command);\n"\
    "}\n"\
    "bool rm_dir(char* dir_path){\n"\
    "\tchar command[COMMAND_MAX_LEN];\n"\
    "\tsnprintf(command, sizeof(command), RMDIR_CMD, dir_path);\n"\
    "\treturn !system(command);\n"\
    "}\n"\
    "bool copy_file(char* src_path, char* dest_path){\n"\
    "\tchar command[COMMAND_MAX_LEN];\n"\
    "\tsnprintf(command, sizeof(command), CP_CMD, src_path, dest_path);\n"\
    "\treturn !system(command);\n"\
    "}\n"\
    "static inline time_t get_mtime(char* file_path) {\n"\
    "\tstruct stat st;\n"\
    "\tif (stat(file_path, &st) == -1) return 0;\n"\
    "\treturn st.st_mtime;\n"\
    "}\n"\
    "bool is_newer(char* new_path, char* old_path){\n"\
    "\ttime_t new_mtime = get_mtime(new_path);\n"\
    "\ttime_t old_mtime = get_mtime(old_path);\n"\
    "\tif (new_mtime == 0 || old_mtime == 0) return true;\n"\
    "\treturn new_mtime > old_mtime;\n"\
    "}\n"\
    "bool run_command(char* command){\n"\
    "\tif (!system(command)) {printf(\"[BUILD] %%s\\n\", command); return true;}\n"\
    "\tprintf(\"[BUILD] \\x1B[31;1;4m%%s\\x1B[0m\\n\", command);\n"\
    "\treturn false;\n"\
    "}\n"\
    "bool run_maqe(Maqe* maqes, size_t maqes_len){\n"\
    "\tstatic bool to_run[MAX_MAQES];\n"\
    "\tbool done = false;\n"\
    "\twhile (!done) {\n"\
    "\t\tsize_t run_count = 0;\n"\
    "\t\tmemset(to_run, false, maqes_len);\n"\
    "\t\tfor (size_t i = 0; i < maqes_len; ++i) {\n"\
    "\t\t\tbool runnable = false;\n"\
    "\t\t\tfor (size_t j = 0; j < maqes[i].targets_count; ++j){\n"\
    "\t\t\t\tif (maqes[i].dependencies_count == 0) { runnable = true; break; }\n"\
    "\t\t\t\tfor (size_t k = 0; k < maqes[i].dependencies_count; ++k) {\n"\
    "\t\t\t\t\tif (!file_exists(maqes[i].dependencies[k])) {runnable = false; break;}\n"\
    "\t\t\t\t\tif (!file_exists(maqes[i].targets[j]) || is_newer(maqes[i].dependencies[k], maqes[i].targets[j])) {runnable = true;}\n"\
    "\t\t\t\t}\n"\
    "\t\t\t\tif (!runnable) break;\n"\
    "\t\t\t}\n"\
    "\t\t\tto_run[i] = runnable;\n"\
    "\t\t\tif (runnable) run_count++;\n"\
    "\t\t}\n"\
    "\t\tif (run_count == 0) { done = true; break; }\n"\
    "\t\tfor (size_t i = 0; i < maqes_len; ++i) {\n"\
    "\t\t\tif (!to_run[i]) continue;\n"\
    "\t\t\tfor (size_t l = 0; l < maqes[i].commands_count; ++l) {\n"\
    "\t\t\t\tif (!run_command(maqes[i].commands[l])) return false;\n"\
    "\t\t\t}\n"\
    "\t\t}\n"\
    "\t}\n"\
    "\treturn true;\n"\
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
    "void print_info(char* message){printf(\"[BUILD] [INFO] %%s\\n\", message);}\n"\
    "void print_error(char* message){printf(\"[BUILD] \\x1B[31;1;4m[ERROR] %%s\\x1B[0m\\n\", message);}\n"

#define BUILD_C_CONTENT \
    "#define PROJECT \"%s\"\n\n"\
    "#include \"build.h\"\n"\
    "#define CC \"gcc\"\n"\
    "#define LINK \"\"\n"\
    "#define FLAGS \"-Wall -Wextra -Werror\"\n\n"\
    "int main(){\n"\
    "    //This is a makefile-like build system with incremental builds\n"\
    "    Maqe_Rule(\n"\
    "        S(\"build/\"PROJECT),\n"\
    "        S(\"src/main.c\"),\n"\
    "        S(CC\" -o build/\"PROJECT\" src/main.c \"LINK\" \"FLAGS)\n"\
    "    );\n"\
    "    Maqe_Run();\n"\
    "    //Alternatively you can run commands manually\n"\
    "    //run_command(CC\" -o build/\"PROJECT\" src/main.c \"LINK\" \"FLAGS);\n"\
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
    if (MKDIR(path) == 0 || errno == EEXIST) return true;
    return false;
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
        if (!file_exists(path)) {
            FILE *config_file = fopen(path, "w");
            if (!config_file) { perror("fopen"); return 1; }
            fprintf(config_file, CONFIG_FILE_CONTENT);
            fclose(config_file);
        }
        snprintf(path, sizeof(path), "%s/src/main.c", argv[2]);
        if(!file_exists(path)){
            FILE *mainc = fopen(path, "w");
            if (!mainc) { perror("fopen main.c"); return 1; }
            fprintf(mainc, MAIN_C_CONTENT);
            fclose(mainc);
        }
        snprintf(path, sizeof(path), "%s/build.h", argv[2]);
        if(!file_exists(path)){
            FILE *buildh = fopen(path, "w");
            if (!buildh) { perror("fopen build.h"); return 1; }
            fprintf(buildh, BUILD_H_CONTENT);
            fclose(buildh);
        }
        snprintf(path, sizeof(path), "%s/build.c", argv[2]);
        if(!file_exists(path)){
            FILE *buildc = fopen(path, "w");
            if (!buildc) { perror("fopen build.c"); return 1; }
            fprintf(buildc, BUILD_C_CONTENT, argv[2]);
            fclose(buildc);
        }
        snprintf(path, sizeof(path), "%s/test.c", argv[2]);
        if(!file_exists(path)){
            FILE *testc = fopen(path, "w");
            if (!testc) { perror("fopen test.c"); return 1; }
            fprintf(testc, TEST_C_CONTENT);
            fclose(testc);
        }
        return 0;
    }
    else if(strcmp(argv[1], "test")==0) {
        FILE* config_file = fopen(".nct/.nct", "r");
        if(!config_file) {printf("[NCT] \x1B[31;1;4mCouldn't open .nct/.nct. Make sure that this is a valid nct project directory or create one using \'nct init\'\x1B[0m\n"); return 1;}
        char test_compile[CONFIG_MAX_LEN]={0};
        char test_binary[CONFIG_MAX_LEN]={0};
        if (!get_config(config_file, "test_compile", test_compile)) {
            printf("[NCT] \x1B[31;1;4mtest_compile configuration not found in .nct/.nct\x1B[0m\n");
            fclose(config_file);
            return 1;
        }
        if (!get_config(config_file, "test_binary", test_binary)) {
            printf("[NCT] \x1B[31;1;4mtest_binary configuration not found in .nct/.nct\x1B[0m\n");
            fclose(config_file);
            return 1;
        }
        fclose(config_file);
        if (!file_exists(test_binary)) {if(system(test_compile)) {printf("[NCT] \x1B[31;1;4mCouldn't run test compile command: %s\x1B[0m\n", test_compile); return 1;}}
        else if (get_mtime("test.c")>get_mtime(test_binary)) if(system(test_compile)) {printf("[NCT] \x1B[31;1;4mCouldn't run test compile command: %s\x1B[0m\n", test_compile); return 1;}
        char run_test_command[COMMAND_MAX_LEN];
        snprintf(run_test_command, sizeof(run_test_command), "%s", test_binary);
        for(int i=2; i<argc; ++i) {
            if (strlen(run_test_command)+strlen(argv[i])+1>COMMAND_MAX_LEN) {printf("[NCT] \x1B[31;1;4mToo many arguments and/or command too long.\x1B[0m\n"); return 1;}
            strcat(run_test_command, " ");
            strcat(run_test_command, argv[i]);
        }
        if(system(run_test_command)) {printf("[NCT] Test failed or returned non-zero exit code\n"); return 1;}
    }
    else if(strcmp(argv[1], "build")==0) {
        FILE* config_file = fopen(".nct/.nct", "r");
        if(!config_file) {printf("[NCT] \x1B[31;1;4mCouldn't open .nct/.nct. Make sure that this is a valid nct project directory or create one using \'nct init\'\x1B[0m\n"); return 1;}
        char build_compile[CONFIG_MAX_LEN]={0};
        char build_binary[CONFIG_MAX_LEN]={0};
        if (!get_config(config_file, "build_compile", build_compile)) {
            printf("[NCT] \x1B[31;1;4mbuild_compile configuration not found\x1B[0m\n");
            fclose(config_file);
            return 1;
        }
        if (!get_config(config_file, "build_binary", build_binary)) {
            printf("[NCT] \x1B[31;1;4mbuild_binary configuration not found\x1B[0m\n");
            fclose(config_file);
            return 1;
        }
        fclose(config_file);
        if (!file_exists(build_binary)) {if(system(build_compile)) {printf("[NCT] \x1B[31;1;4mCouldn't run build compile command: %s\x1B[0m\n", build_compile); return 1;}}
        else if (get_mtime("build.c")>get_mtime(build_binary)) if(system(build_compile)) {printf("[NCT] \x1B[31;1;4mCouldn't run build compile command: %s\x1B[0m\n", build_compile); return 1;}
        char run_build_command[COMMAND_MAX_LEN];
        snprintf(run_build_command, sizeof(run_build_command), "%s", build_binary);
        for(int i=2; i<argc; ++i) {
            if (strlen(run_build_command)+strlen(argv[i])+1>COMMAND_MAX_LEN) {printf("[NCT] \x1B[31;1;4mToo many arguments and/or command too long.\x1B[0m\n"); return 1;}
            strcat(run_build_command, " ");
            strcat(run_build_command, argv[i]);
        }
        if(system(run_build_command)) {printf("[NCT] Build failed or returned non-zero exit code\n"); return 1;}
    }
    else {printf("Unknown command was given. Run 'nct help' for more information.\n"); return 1;}
    return 0;
}
