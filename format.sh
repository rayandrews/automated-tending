#!/bin/bash

find app -iname *.h -o -iname *.cpp | xargs clang-format -i
find include -iname *.h -o -iname *.cpp | xargs clang-format -i
find src -iname *.h -o -iname *.cpp | xargs clang-format -i