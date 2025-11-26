# agatetepe-gui

This is a front end for [agatetepe](https://github.com/RickStanley/agatetepe).

If on Linux:

```bash
podman run --rm -it \
                -e XDG_RUNTIME_DIR=/tmp \
                -e WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
                -v $XDG_RUNTIME_DIR/$WAYLAND_DISPLAY:/tmp/$WAYLAND_DISPLAY \
                -v $PWD/src:/app/src \
                -v $PWD/include:/app/include \
                -v $PWD/CMakeLists.txt:/app/CMakeLists.txt \
                -e COLORTERM=truecolor \
                --security-opt=label=type:container_runtime_t \
                --name wxstc-requests-dev \
                localhost/wxstc-requests:dev

cd /app

cmake -B build -DCMAKE_BUILD_TYPE=Debug

cmake --build build

./build/wxstc-editor
```