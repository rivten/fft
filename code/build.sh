# NOTE(hugo) : I would like to use -pedantic-errors but anonymous structs are not a thing in C++11
# whereas it is in C11... who knows.
# NOTE(hugo) : I had to delete -Wcast-qual -Wshadow to compile stb libs

CODE_PATH="$(dirname "$0")"
CTIME_EXEC="$CODE_PATH/../../ctime/ctime"
CTIME_TIMING_FILE="$CODE_PATH/../build/compile_time_vulkan.ctm"

if [ ! -f "$CTIME_EXEC" ]; then
	cc -O2 -Wno-unused-result "$CODE_PATH/../../ctime/ctime.c" -o "$CTIME_EXEC"
fi

$CTIME_EXEC -begin "$CTIME_TIMING_FILE"

CommonFlags="-g -std=c++11 -Werror -Wall -Wextra -Wcast-align -Wmissing-noreturn -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-include-dirs -Wno-old-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-promo -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option -Wno-write-strings -Wno-absolute-value -Wno-cast-align -Wno-unused-parameter -lm"
CommonFlags+=" -I ../../rivten/ -I ../../stb/ -I ../../imgui/"

if [ -n "$(command -v clang++)" ]
then
	CXX=clang++
	CommonFlags+=" -Wno-missing-braces -Wno-null-dereference -Wno-self-assign"
else
  CXX=c++
  CommonFlags+=" -Wno-unused-but-set-variable"
fi

CommonLinkerFlags="-l SDL2 -l vulkan -l X11-xcb"

mkdir -p "$CODE_PATH/../build"
pushd "$CODE_PATH/../build"

$CXX $CommonFlags ../code/fft.cpp $CommonLinkerFlags -o fft

popd

$CTIME_EXEC -end "$CTIME_TIMING_FILE"
