#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#define CONF_FILE ".nct/.nct"
#define PROJ_NAME_LEN 16
#define CONF_LEN 128
#define MTIME_LEN 32

static inline char exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r"))) {fclose(file); return 1;}
    return 0;
}

static inline time_t get_mtime(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == -1) return 0;
    return st.st_mtime;
}
static inline int mkdir_safe(const char *path) {
    #ifndef WIN32
        if (mkdir(path, 0755) == -1 && errno != EEXIST) {
    #endif
    #ifdef WIN32
        if (mkdir(path) == -1 && errno != EEXIST) {
    #endif
        perror("mkdir");
        return 0;
    }
    return 1;
}

int main(int argc, char** argv){
    if(argc==1) {printf("No command was given. Run 'nct help' for more information.\n"); return 1;}
    else if(strcmp(argv[1], "help")==0) {printf("Usage: nct [COMMAND] [ARGS]\n\n"
        "Commands:\n"
        "\tinit    [NAME] Initializes a project with a given name and make .nct/.nct\n"
        "\tbuild   [ARGS] Builds project using build.c\n"
        "\trebuild [ARGS] Force rebuilds project using build.c\n"
        "\trun     [ARGS] Run the built executable\n"
        "\ttest    [ARGS] Build and run test.c\n"
        "\thelp    Print this message\n\n"
        ".nct file structure: \n"
        "\t<compile test command>\n"
        "\t<run test command>\n"
        "\t<compile build command>\n"
        "\t<run build command>\n"
        "\t<run bin command>\n"
        "\t<last modified test>\n"
        "\t<last modified build>\n"); return 0;}
    else if(strcmp(argv[1], "init")==0) {
        if(argc==2) {printf("No project name was given. Run 'nct help' for more information.\n"); return 1;}
        if(strlen(argv[2])>PROJ_NAME_LEN) {printf("Project name too long (max %d chars).\n", PROJ_NAME_LEN); return 1;}
        if (!mkdir_safe(argv[2])) return 1;
        char path[256];
        snprintf(path, sizeof(path), "%s/.nct", argv[2]);
        if (!mkdir_safe(path)) return 1;
        snprintf(path, sizeof(path), "%s/bin", argv[2]);
        if (!mkdir_safe(path)) return 1;
        snprintf(path, sizeof(path), "%s/src", argv[2]);
        if (!mkdir_safe(path)) return 1;
        snprintf(path, sizeof(path), "%s/.nct/.nct", argv[2]);
        FILE *conf = fopen(path, "w");
        if (!conf) { perror("fopen"); return 1; }
        #ifndef WIN32
            fprintf(conf, 
                "gcc ./test.c -o ./.nct/test -Wall -Wextra\n"
                "./.nct/test\n"
                "gcc ./build.c -o ./.nct/build -Wall -Wextra\n"
                "./.nct/build\n"
                "./bin/%s\n"
                "0\n0\n", argv[2]);
        #endif
        #ifdef WIN32
            fprintf(conf, 
                "gcc test.c -o .nct\\test -Wall -Wextra\n"
                ".nct\\test\n"
                "gcc build.c -o .nct\\build -Wall -Wextra\n"
                ".nct\\build\n"
                "bin\\%s\n"
                "0\n0\n", argv[2]); 
        #endif
        fclose(conf);
        snprintf(path, sizeof(path), "%s/src/common.h", argv[2]);
        FILE *common = fopen(path, "w");
        fprintf(common, 
            "#pragma once\n"
            "#include <stdint.h>\n");
        fclose(common);
        snprintf(path, sizeof(path), "%s/src/main.c", argv[2]);
        FILE *mainc = fopen(path, "w");
        fprintf(mainc,
            "#include \"common.h\"\n"
            "#include <stdio.h>\n\n"
            "int main(){\n"
            "    printf(\"Hello World\\n\");\n"
            "    return 0;\n"
            "}\n");
        fclose(mainc);
        snprintf(path, sizeof(path), "%s/build.c", argv[2]);
        FILE *buildc = fopen(path, "w");
        fprintf(buildc,
            "#include <stdlib.h>\n\n"
            "#define COMPILER \"gcc\"\n"
            "#define SRC \"./src/main.c\"\n"
            "#define BIN \"./bin/%s\"\n"
            "#define FLAGS \"-Wall -Wextra -Werror -O3\"\n\n"
            "int main(){\n"
            "    system(COMPILER\" \"SRC\" -o \"BIN\" \" FLAGS);\n"
            "    return 0;\n"
            "}\n", argv[2]);
        fclose(buildc);
        snprintf(path, sizeof(path), "%s/test.c", argv[2]);
        FILE *testc = fopen(path, "w");
        fprintf(testc,
            "#include <stdio.h>\n\n"
            "int main(){\n"
            "    printf(\"No tests written\\n\");\n"
            "    return 0;\n"
            "}\n");
        fclose(testc);
        return 0;
    }
    char compile_test_command[CONF_LEN], run_test_command[CONF_LEN],
    compile_build_command[CONF_LEN], run_build_command[CONF_LEN],
    run_bin_command[CONF_LEN],
    last_modified_test[MTIME_LEN], last_modified_build[MTIME_LEN], 
    get_last_modified_test[MTIME_LEN], get_last_modified_build[MTIME_LEN];
    FILE* file;
    if(strcmp(argv[1], "run")==0) {
        file = fopen("./.nct/.nct", "r");
        if(!file) {printf("Couldn't open .nct/.nct. Make sure that a valid .nct/.nct exist or create one using \'nct init\'\n"); return 1;}
        if(!fgets(compile_test_command, CONF_LEN, file) || !fgets(run_test_command, CONF_LEN, file) || !fgets(compile_build_command, CONF_LEN, file) || !fgets(run_build_command, CONF_LEN, file) || !fgets(run_bin_command, CONF_LEN, file)) {printf("Couldn't read .nct/.nct properly. Make sure that a valid .nct/.nct exist or create one using \'nct init\'\n"); fclose(file); return 1;}
        fclose(file);
        run_bin_command[strlen(run_bin_command)-1]=0;
        if (!exists(run_bin_command)) {printf("%s was not found. Run \'nct build ...\' if needed.\n", run_bin_command); return 0;}
        for(int i=2; i<argc; ++i) {
            if (strlen(run_bin_command)+strlen(argv[i])+1>CONF_LEN) {printf("Too many arguments and/or command too long.\n"); return 1;}
            strcat(run_bin_command, " ");
            strcat(run_bin_command, argv[i]);
        }
        if(system(run_bin_command)) {printf("Couldn't run bin command properly.\n"); return 1;}
        return 0;
    }
    else if(strcmp(argv[1], "test")==0) {
        file = fopen("./.nct/.nct", "r");
        if(!file) {printf("Couldn't open .nct/.nct. Make sure that a valid .nct/.nct exist or create one using \'nct init\'\n"); return 1;}
        if(!fgets(compile_test_command, CONF_LEN, file) || !fgets(run_test_command, CONF_LEN, file) || !fgets(compile_build_command, CONF_LEN, file) || !fgets(run_build_command, CONF_LEN, file) || !fgets(run_bin_command, CONF_LEN, file) || !fgets(last_modified_test, MTIME_LEN, file) || !fgets(last_modified_build, MTIME_LEN, file)) {printf("Couldn't read .nct/.nct properly. Make sure that a valid .nct/.nct exist or create one using \'nct init\'\n"); fclose(file); return 1;}
        fclose(file);
        snprintf(get_last_modified_test, MTIME_LEN, "%ld", (long)get_mtime("test.c"));
        file = fopen("./.nct/.nct", "w");
        if(!file) {printf("Couldn't open .nct/.nct. Make sure that a valid .nct/.nct exist or create one using \'nct init\'\n"); return 1;}
        fprintf(file, "%s%s%s%s%s", compile_test_command, run_test_command, compile_build_command, run_build_command, run_bin_command);
        last_modified_test[strlen(last_modified_test)-1]=0;
        run_test_command[strlen(run_test_command)-1]=0;
        if(strcmp(last_modified_test, get_last_modified_test) | !exists(run_test_command)) if(system(compile_test_command)) {
            printf("Couldn't run compile test command properly.\n");
            fprintf(file, "%s\n%s", last_modified_test, last_modified_build);
            fclose(file); return 1;
        }
        fprintf(file, "%s\n%s", get_last_modified_test, last_modified_build);
        fclose(file);
        for(int i=2; i<argc; ++i) {
            if (strlen(run_test_command)+strlen(argv[i])+1>CONF_LEN) {printf("Too many arguments and/or command too long.\n"); return 1;}
            strcat(run_test_command, " ");
            strcat(run_test_command, argv[i]);
        }
        if(system(run_test_command)) {printf("Couldn't run test command properly.\n"); return 1;}
    }
    else if(strcmp(argv[1], "build")==0) {
        file = fopen("./.nct/.nct", "r");
        if(!file) {printf("Couldn't open .nct/.nct. Make sure that a valid .nct/.nct exist or create one using \'nct init\'\n"); return 1;}
        if(!fgets(compile_test_command, CONF_LEN, file) || !fgets(run_test_command, CONF_LEN, file) || !fgets(compile_build_command, CONF_LEN, file) || !fgets(run_build_command, CONF_LEN, file) || !fgets(run_bin_command, CONF_LEN, file) || !fgets(last_modified_test, MTIME_LEN, file) || !fgets(last_modified_build, MTIME_LEN, file)) {printf("Couldn't read .nct/.nct properly. Make sure that a valid .nct/.nct exist or create one using \'nct init\'\n"); fclose(file); return 1;}
        fclose(file);
        snprintf(get_last_modified_build, MTIME_LEN, "%ld", (long)get_mtime("build.c"));
        file = fopen("./.nct/.nct", "w");
        if(!file) {printf("Couldn't open .nct/.nct. Make sure that a valid .nct/.nct exist or create one using \'nct init\'\n"); return 1;}
        fprintf(file, "%s%s%s%s%s", compile_test_command, run_test_command, compile_build_command, run_build_command, run_bin_command);
        last_modified_build[strlen(last_modified_build)-1]=0;
        run_build_command[strlen(run_build_command)-1]=0;
        if(strcmp(last_modified_build, get_last_modified_build) | !exists(run_build_command)) if(system(compile_build_command)) {
            printf("Couldn't run compile build command properly.\n");
            fprintf(file, "%s%s\n", last_modified_test, last_modified_build);
            fclose(file); return 1;
        }
        fprintf(file, "%s%s\n", last_modified_test, get_last_modified_build);
        fclose(file);
        for(int i=2; i<argc; ++i) {
            if (strlen(run_build_command)+strlen(argv[i])+1>CONF_LEN) {printf("Too many arguments and/or command too long.\n"); return 1;}
            strcat(run_build_command, " ");
            strcat(run_build_command, argv[i]);
        }
        if(system(run_build_command)) {printf("Couldn't run build command properly.\n"); return 1;}
    }
    else if(strcmp(argv[1], "rebuild")==0) {
        file = fopen("./.nct/.nct", "r");
        if(!file) {printf("Couldn't open .nct/.nct. Make sure that a valid .nct/.nct exist or create one using \'nct init\'\n"); return 1;}
        if(!fgets(compile_test_command, CONF_LEN, file) || !fgets(run_test_command, CONF_LEN, file) || !fgets(compile_build_command, CONF_LEN, file) || !fgets(run_build_command, CONF_LEN, file) || !fgets(run_bin_command, CONF_LEN, file) || !fgets(last_modified_test, MTIME_LEN, file) || !fgets(last_modified_build, MTIME_LEN, file)) {printf("Couldn't read .nct/.nct properly. Make sure that a valid .nct/.nct exist or create one using \'nct init\'\n"); fclose(file); return 1;}
        fclose(file);
        if(system(compile_build_command)) {printf("Couldn't run compile build command properly.\n"); return 1;}
        run_build_command[strlen(run_build_command)-1]=0;
        for(int i=2; i<argc; ++i) {
            if (strlen(run_build_command)+strlen(argv[i])+1>CONF_LEN) {printf("Too many arguments and/or command too long.\n"); return 1;}
            strcat(run_build_command, " ");
            strcat(run_build_command, argv[i]);
        }
        if(system(run_build_command)) {printf("Couldn't run build command properly.\n"); return 1;}
    }
    else {printf("Unknown command was given. Run 'nct help' for more information.\n"); return 1;}
    return 0;
}

