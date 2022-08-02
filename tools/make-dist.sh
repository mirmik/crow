docker build . -t arm32v7/crow -f tools/dockerfile.arch --build-arg ARCH=arm32v7
docker build . -t arm64v8/crow -f tools/dockerfile.arch --build-arg ARCH=arm64v8
docker build . -t crow -f tools/dockerfile.host

docker run --rm --entrypoint cat crow /root/crow/libcrow.so > dist/libcrow.so
docker run --rm --entrypoint cat arm32v7/crow /root/crow/libcrow.so > dist/arm32v7/libcrow.so
docker run --rm --entrypoint cat arm64v8/crow /root/crow/libcrow.so > dist/arm64v8/libcrow.so

docker run --rm --entrypoint cat crow /root/crow/libcrow.a > dist/libcrow.a
docker run --rm --entrypoint cat arm32v7/crow /root/crow/libcrow.a > dist/arm32v7/libcrow.a
docker run --rm --entrypoint cat arm64v8/crow /root/crow/libcrow.a > dist/arm64v8/libcrow.a