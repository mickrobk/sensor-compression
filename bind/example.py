# You can spin up a python instance to try this with:
#    docker run --rm -it  $(docker build -q -f docker/py/Dockerfile .) python3
#
# You can either copy/paste the following or run `import bind.example`

import json
from types import SimpleNamespace
import sensor_compress as sensor
from datetime import datetime, timezone
import pytz

json_compressed_readings = '{"frames":[{"side_channel":[0,10,10,6686828444,10,10],"times":[40,11,179,52,139,178,5,112],"values":[40,11,179,52,139,178,5,112]}],"header":{"frame_size":1000,"max":4294967295,"min":0,"name":"example time sensor","resolution_bits":32,"session_id":"ddfbe2e2-4de3-431e-9015-39d4f8afdd9c","start_time_steady":6686828556,"start_time_utc":1723353284037,"time_compressions":[2,3,0],"value_compressions":[2,3,0],"version":1}}'
expected_json_decompressed_readings = '{"header":{"frame_size":1000,"max":4294967295,"min":0,"name":"example time sensor","resolution_bits":32,"session_id":"ddfbe2e2-4de3-431e-9015-39d4f8afdd9c","start_time_steady":6686828556,"start_time_utc":1723353284037,"time_compressions":[2,3,0],"value_compressions":[2,3,0],"version":1},"values":[{"t":6686828444,"value":0},{"t":6686828455,"value":11},{"t":6686828466,"value":22},{"t":6686828476,"value":32},{"t":6686828487,"value":43},{"t":6686828500,"value":56},{"t":6686828511,"value":67},{"t":6686828523,"value":79},{"t":6686828534,"value":90},{"t":6686828544,"value":100}]}'

json_decompressed_readings = sensor.decompressSensorReadings(json_compressed_readings)
readings = json.loads(json_decompressed_readings, object_hook=lambda d: SimpleNamespace(**d))
readings_utc = datetime.fromtimestamp(readings.header.start_time_utc / 1000, tz=timezone.utc)

assert(json_decompressed_readings == expected_json_decompressed_readings)
mountain_zone = pytz.timezone('America/Denver')

print('the decompressed object has {} values recorded from session id {}'.format(len(readings.values), readings.header.session_id))
print('those values are: {}'.format([x.value for x in readings.values]))
print('it was recorded at {} MT'.format(readings_utc.astimezone(mountain_zone)))
