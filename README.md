# sensor-compression

Lightweight utilities for compressing sensor time series data.

Includes c++ implementation of Simple8b, delta, and zig-zag encoding/decoding.

Forked from https://github.com/naturalplasmoid/simple8b-timeseries-compression, adapted from https://github.com/lemire/FastPFor, (Apache License Version 2.0)

# TODO
 - memory safe API
 - Fuzz testing

# Python support

docker run --rm -it  $(docker build -q -f bind/docker/Dockerfile .) python3
```
import sensor_compress as sensor

## OR import bind.example
```

# Building
cmake . -B build -G Ninja -DSENSOR_TEST=ON -DSENSOR_BIND=ON  && ninja -C build

### Running tests
./build/lib/sensor_test 

#### Example test runs
// Rebuild and run only sensor-compress json tests
clear && cmake . -DSENSOR_TEST=ON -DSENSOR_BIND=ON -B build -G Ninja  && ninja -C build  && ./build/lib/sensor_test 

// Filter like so: --gtest_filter="JsonTest*"