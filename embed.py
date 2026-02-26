with open("build.h", "r") as f:
    content = f.read()
escaped = content.replace("\r\n", "\n").replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n\"\n\"")
with open("build_h_embed.h", "w") as out:
    out.write(f'const char* build_h_content =\n"{escaped}";\n')

with open("build.c", "r") as f:
    content = f.read()
escaped = content.replace("\r\n", "\n").replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n\"\n\"")
with open("build_c_embed.h", "w") as out:
    out.write(f'const char* build_c_content =\n"{escaped}";\n')