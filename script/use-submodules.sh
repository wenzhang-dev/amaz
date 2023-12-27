#!/bin/bash

set -e

git submodule update --init --recursive
git submodule foreach -q --recursive \
  'git checkout $(git config -f $toplevel/.gitmodules submodule.$name.tag || git config -f $toplevel/.gitmodules submodule.$name.branch)'


