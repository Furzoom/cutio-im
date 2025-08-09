#!/bin/bash

set -e

git submodule init
git submodule update --init --remote
git submodule foreach -q --recursive \
    "tag=\$(git config -f \$toplevel/.gitmodules submodule.\$name.tag);
    if [ @\$tag != '@' ]; then
      git checkout -q tags/\$tag;
    fi"
