Get-ChildItem -Recurse -Include *.cpp,*.h,*.hpp,*.cxx,*.cc |
Where-Object {
    $_.FullName -notmatch '\\build\\' -and
    $_.FullName -notmatch '\\third_party\\' -and
    $_.FullName -notmatch '\\external\\' -and
    $_.FullName -notmatch '\\vendor\\' -and
    $_.FullName -notmatch '\\generated\\'
} | ForEach-Object {
    clang-format -i $_.FullName
}
