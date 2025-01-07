SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

cd "$SCRIPT_DIR"

if [ -d "build" ]; then
    rm -r build
fi

mkdir -p build && cd build

cmake ../src
make -j8