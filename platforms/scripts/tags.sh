#!/bin/bash

# This script calls ctags and cscope to build tags info for Vim.
# Uses some sed/awk tricks to get tags where they would otherwise be obscured by macros.
# Run at the root of the project.

FILES="deadking/game_structs.h mcdiddy/game_structs.h engine/engine_structs.h"

for file in $FILES
do
    mv $file{,.bak}
    sed -e 's/#define *TYPE *\(.*\)/TYPE(\1(\/\/~:~\0/'                        \
        -e 's/^ *EXPOSE(\([^,]*\),\([^,]*\),\([^,]*\)).*$/\1 \2\3; \/\/~:~\0/' \
        $file.bak                                                              \
        | awk -F'('                                                            \
        '                  { nom=1; }
        $1 ~ "^ *TYPE$"    { nom=0; type=$2; print "typedef struct " $2 "_t { //" $0; }
        $1 ~ "^ *STRUCT$" ||
        $1 ~ "^ *EXPOSE$" ||
        $1 ~ "^ *HIDE$"   ||
        $1 ~ "^ *)"        { nom=0; print "//" $0; }
        $1 ~ "endstruct.h" { nom=0; print "} " type "_t;"; }
        nom                { print $0; }
        ' >$file
done

ctags {engine,deadking,mcdiddy}/*.[ch]
cscope -b -I engine -I deadking -I mcdiddy -s engine -s deadking -s mcdiddy

sed -e 's/\/\^.*\\\/\\\/~:~/\/^/' -i tags

for file in $FILES
do
    mv $file{.bak,}
done
