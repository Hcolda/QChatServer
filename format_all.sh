#!/bin/bash

EXTENSIONS=("*.cpp" "*.h" "*.hpp" "*.cxx" "*.cc" "*.c++" "*.h++")
EXCLUDE_DIRS=("build/" "third_party/" "external/" "vendor/" "generated/")
CLANG_FORMAT="clang-format"

exclude_args=()
for dir in "${EXCLUDE_DIRS[@]}"; do
    exclude_args+=(-not -path "./${dir}*")
done

find . -type f \( \
    -name "${EXTENSIONS[0]}" \
    $(printf -- '-o -name "%s" ' "${EXTENSIONS[@]:1}") \
\) "${exclude_args[@]}" -exec $CLANG_FORMAT -i {} \;

echo "Formatted all existing C++ files"
