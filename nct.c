#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>

#define COMMAND_MAX_LEN 512

#define VERSION "3.0"

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
    "gcc build.c -o .nct/build\n"\
    ".nct/build\n"\
    "gcc test.c -o .nct/test\n"\
    ".nct/test\n"\

#define MAIN_C_CONTENT \
    "#include <stdio.h>\n"\
    "int main(){\n"\
    "    printf(\"Hello World\\n\");\n"\
    "    return 0;\n"\
    "}\n"

#define TEST_C_CONTENT \
    "#include <stdio.h>\n"\
    "int main(){\n"\
    "    printf(\"No tests written\\n\");\n"\
    "    return 0;\n"\
    "}\n"

#include "build_h_embed.h"
#include "build_c_embed.h"

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
    #define CHDIR(path) _chdir(path)
    #define ERROR_PRINT_SET ""
    #define ERROR_PRINT_UNSET ""
#else
    #include <unistd.h>
    #define MKDIR(path) mkdir(path, 0755)
    #define CHDIR(path) chdir(path)
    #define ERROR_PRINT_SET "\x1B[31;1;4m"
    #define ERROR_PRINT_UNSET "\x1B[0m"
#endif
static inline bool nct_mkdir(const char *path) {
    if (MKDIR(path) == 0 || errno == EEXIST) return true;
    return false;
}
static inline bool nct_chdir(const char *path) {
    if (CHDIR(path) == 0) return true;
    return false;
}
static inline void nct_print_error(const char* msg, const char* arg) {
    printf("[NCT] "ERROR_PRINT_SET"%s%s"ERROR_PRINT_UNSET"\n", msg, arg);
}

typedef enum {BUILD_COMPILE = 0, BUILD_BINARY, TEST_COMPILE, TEST_BINARY} ConfigType;
static inline bool get_config(FILE* file, ConfigType config_type, char* config_buffer){
    size_t p = 0; char c;
    for (size_t i = 0; i < config_type; ++i) while (true) {c = fgetc(file); if (c == EOF || c == '\n') break;}
    while (true){
        c = fgetc(file); if (c == EOF || c == '\n') break;
        *(config_buffer+p) = c; ++p; if (p >= COMMAND_MAX_LEN - 1) break;
    }
    if (p == COMMAND_MAX_LEN - 1) *(config_buffer+p) = '\0';
    fseek(file, 0, SEEK_SET);
    return p!=0;
}

int main(int argc, char** argv){
    if(argc==1) {
        printf("No command was given. Run 'nct help' for more information.\n"); 
        return 1;
    }
    else if(strcmp(argv[1], "version")==0) {
        printf(ASCII_LOGO"\n                NCT VERSION: "VERSION"\n\n"); 
        return 0;
    }
    else if(strcmp(argv[1], "help")==0) {
        printf(HELP_MESSAGE); 
        return 0;
    }
    else if(strcmp(argv[1], "init")==0) {
        if(argc==2) {printf("No project name was given. Run 'nct help' for more information.\n"); return 1;}
        if (!nct_mkdir(argv[2])) return 1;
        if (!nct_chdir(argv[2])) return 1;
        if (!nct_mkdir(".nct")) return 1;
        if (!nct_mkdir("build")) return 1;
        if (!nct_mkdir("lib")) return 1;
        if (!nct_mkdir("src")) return 1;
        if (!file_exists(".nct/.nct")) {
            FILE *config_file = fopen(".nct/.nct", "w");
            if (!config_file) { perror("fopen .nct/.nct"); return 1; }
            fprintf(config_file, CONFIG_FILE_CONTENT);
            fclose(config_file);
        }
        if(!file_exists("src/main.c")){
            FILE *mainc = fopen("src/main.c", "w");
            if (!mainc) { perror("fopen main.c"); return 1; }
            fprintf(mainc, MAIN_C_CONTENT);
            fclose(mainc);
        }
        if(!file_exists("build.h")){
            FILE *buildh = fopen("build.h", "w");
            if (!buildh) { perror("fopen build.h"); return 1; }
            fputs(build_h_content, buildh);
            fclose(buildh);
        }
        if(!file_exists("build.c")){
            FILE *buildc = fopen("build.c", "w");
            if (!buildc) { perror("fopen build.c"); return 1; }
            fprintf(buildc, build_c_content, argv[2]);
            fclose(buildc);
        }
        if(!file_exists("test.c")){
            FILE *testc = fopen("test.c", "w");
            if (!testc) { perror("fopen test.c"); return 1; }
            fprintf(testc, TEST_C_CONTENT);
            fclose(testc);
        }
        return 0;
    }
    else if(strcmp(argv[1], "test")==0) {
        FILE* config_file = fopen(".nct/.nct", "r");
        if(!config_file) {nct_print_error("Couldn't open .nct/.nct. Make sure that this is a valid nct project directory or create one using \'nct init\'\n", ""); return 1;}
        char test_compile[COMMAND_MAX_LEN]={0};
        if (!get_config(config_file, TEST_COMPILE, test_compile)) {nct_print_error("Couldn't read test compile config", ""); return 1;}
        char test_binary[COMMAND_MAX_LEN]={0};
        if (!get_config(config_file, TEST_BINARY, test_binary)) {nct_print_error("Couldn't read test binary config", ""); return 1;}
        fclose(config_file);
        if (!file_exists(test_binary)) {if(system(test_compile)) {nct_print_error("Couldn't run test compile command: \n", test_compile); return 1;}}
        else if (get_mtime("test.c")>get_mtime(test_binary)) if(system(test_compile)) {nct_print_error("Couldn't run test compile command: \n", test_compile); return 1;}
        for(int i=2; i<argc; ++i) {
            if (strlen(test_binary)+strlen(argv[i])+2>COMMAND_MAX_LEN) {nct_print_error("Too many arguments and/or command too long.\n", ""); return 1;}
            strcat(test_binary, " "); strcat(test_binary, argv[i]);
        }
        if(system(test_binary)) {nct_print_error("Test failed or returned non-zero exit code\n", ""); return 1;}
        return 0;
    }
    else if(strcmp(argv[1], "build")==0) {
        FILE* config_file = fopen(".nct/.nct", "r");
        if(!config_file) {nct_print_error("Couldn't open .nct/.nct. Make sure that this is a valid nct project directory or create one using \'nct init\'\n", ""); return 1;}
        char build_compile[COMMAND_MAX_LEN]={0};
        if (!get_config(config_file, BUILD_COMPILE, build_compile)) {nct_print_error("Couldn't read build compile config", ""); return 1;}
        char build_binary[COMMAND_MAX_LEN]={0};
        if (!get_config(config_file, BUILD_BINARY, build_binary)) {nct_print_error("Couldn't read build binary config", ""); return 1;}
        fclose(config_file);
        if (!file_exists(build_binary)) {if(system(build_compile)) {nct_print_error("Couldn't run build compile command: \n", build_compile); return 1;}}
        else if (get_mtime("build.c")>get_mtime(build_binary)) if(system(build_compile)) {nct_print_error("Couldn't run build compile command: \n", build_compile); return 1;}
        for(int i=2; i<argc; ++i) {
            if (strlen(build_binary)+strlen(argv[i])+2>COMMAND_MAX_LEN) {nct_print_error("Too many arguments and/or command too long.\n", ""); return 1;}
            strcat(build_binary, " "); strcat(build_binary, argv[i]);
        }
        if(system(build_binary)) {nct_print_error("Build failed or returned non-zero exit code\n", ""); return 1;}
        return 0;
    }
    else {printf("Unknown command was given. Run 'nct help' for more information.\n"); return 1;}
    return 0;
}
