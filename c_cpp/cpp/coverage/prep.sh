# catch unix signals! handler { exit(0); }
g++ --coverage -g -O0 main.cpp # -fprofile-dir="/tmp"

# run prg do something and shutdown

# run at the top of the project
lcov --capture --directory . --output-file coverage.info # --gcov-tool /usr/bin
# html page
genhtml coverage.info --output-directory Coverage-Html
# open `pwd`/Coverage-Html/index.html
