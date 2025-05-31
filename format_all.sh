#!/bin/bash
# 文件名: format_all.sh

# 配置部分
EXTENSIONS=("*.cpp" "*.h" "*.hpp" "*.cxx" "*.cc" "*.c++" "*.h++")
EXCLUDE_DIRS=("build/" "third_party/" "external/" "vendor/" "generated/")
CLANG_FORMAT="clang-format"

# 创建排除参数
exclude_args=()
for dir in "${EXCLUDE_DIRS[@]}"; do
    exclude_args+=(-not -path "./${dir}*")
done

# 查找并格式化文件
find . -type f \( \
    -name "${EXTENSIONS[0]}" \
    $(printf -- '-o -name "%s" ' "${EXTENSIONS[@]:1}") \
\) "${exclude_args[@]}" -exec $CLANG_FORMAT -i {} \;

echo "Formatted all existing C++ files"
