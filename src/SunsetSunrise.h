/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2025 Sam Groveman
* 
* External libraries needed:
* ArduinoJSON: https://arduinojson.org/
* Webhook: https://github.com/FabricaIO/util-Webhook
* Sunset: https://github.com/buelowp/sunset

* Contributors: Sam Groveman
*/

#pragma once
#include <TimeInterface.h>
#include <Webhook.h>
#include <DeviceConfig.h>
#include <Configuration.h>
#include <ArduinoJson.h>
#include <sunset.h>

/// @brief Used to obtain the sunrise and sunset times for a given location
class SunsetSunrise : public DeviceConfig {
	public:
		/// @brief Configuration of sunrise and sunset settings
		struct {
			/// @brief Latitude of the location for sunset/sunrise
			double lat;

			/// @brief Longitude of the location for sunset/sunrise
			double lng;

			/// @brief Timezone of the location for sunset/sunrise API (see https://en.wikipedia.org/wiki/List_of_tz_database_time_zones)
			String timezone_string;

			/// @brief Offset of the timezone from GMT in seconds for sunset/sunrise calculations
			int timezone_offset_seconds;

			/// @brief Use the locally set timezone offset for sunset/sunrise calculations
			bool automatic_timezone_offset;
		} SunriseSunset_config;

		/// @brief The sunrise and sunset times in hours and minutes past midnight
		struct {
			/// @brief The hour of the sunrise
			int sunrise_hour;

			/// @brief The minute of the sunrise
			int sunrise_minute;

			/// @brief The hour of the sunset
			int sunset_hour;

			/// @brief The minute of the sunset
			int sunset_minute;

			/// @brief When the sunrise and sunset times were last updated
			String last_updated;
		} times;

		SunsetSunrise(String ConfigFile = "SunsetSunrise.json");
		bool begin();
		String getConfig();
		bool setConfig(String config, bool save);;
		bool getTimes();

	protected:
		/// @brief Webhook for sunset
		Webhook sunset_hook{"https://api.sunrise-sunset.org/json"};

		/// @brief Sunset object for calculating sunset/sunrise times
		SunSet sunset;

		/// @brief Full path to config file
		String config_path;

		/// @brief The number of times the API failed
		int API_fail_count = 0;
};