# sensor-compression

Lightweight utilities for compressing sensor time series data.

Includes c++ implementation of Simple8b, delta, and zig-zag encoding/decoding.

Forked from https://github.com/naturalplasmoid/simple8b-timeseries-compression, adapted from https://github.com/lemire/FastPFor, (Apache License Version 2.0)

# TODO
 - memory safe API
 - Fuzz testing

# Python support

docker run --rm -it  $(docker build -q -f docker/py/Dockerfile .) python3
```
import sensor_compress
```

# Analysis

```
cd <sensor_compress...>
docker compose --profile dev up --build 
```
Access via the link output in the terminal

To get a shell in the analysis server:
```
# Must have the container up
docker compose --profile dev exec analyze sh
```

# Building
cmake . -B build -G Ninja  && ninja -C build

### Running tests
./build/sensor_compress/lib/sensor_test

#### Example test runs
// Rebuild and run only sensor-compress json tests
ninja -C build && ./build/sensor_compress/lib/sensor_test --gtest_filter="JsonTest*"