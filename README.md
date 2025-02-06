# util-SunsetSunrise
 Utility to obtain sunrise and sunset times

Can use either API or local calculations to get times. When using API, after three consecutive API failures, will fallback to local calculations. Can get times in any specified location converted to the time in any specified timezone.

Current API used: https://sunrisesunset.io/api/

## Configuration Description
* `lat` The latitude of the location for sunset/sunrise
* `lng` The longitude of the location for sunset/sunrise
* `timezone_string` Timezone of the location for sunset/sunrise API (see https://en.wikipedia.org/wiki/List_of_tz_database_time_zones)
* `timezone_offset_seconds` Timezone offset from GMT in seconds, used for sunset/sunrise calculations
* `automatic_timezone_offset` If true, will use the timezone offset on the device (usually from NTP) and ignore the manually entered offset