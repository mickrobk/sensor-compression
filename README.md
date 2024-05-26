# sensor-compression

Lightweight utilities for compressing sensor time series data.

Includes c++ implementation of Simple8b, delta, and zig-zag encoding/decoding.

Forked from https://github.com/naturalplasmoid/simple8b-timeseries-compression, adapted from https://github.com/lemire/FastPFor, (Apache License Version 2.0)

# Analysis

```
cd <sensor_compress...>
COMPOSE_DOCKER_CLI_BUILD=1 DOCKER_BUILDKIT=1  docker-compose up --build analyze
```
Access via the link output in the terminal