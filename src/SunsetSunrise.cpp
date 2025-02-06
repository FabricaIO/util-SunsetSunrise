#include "SunsetSunrise.h"

/// @brief Constructor for SunsetSunrise
/// @param ConfigFile The configuration file for the SunsetSunrise object
SunsetSunrise::SunsetSunrise(String ConfigFile) {
	config_path = "/settings/util/" + ConfigFile;
}

/// @brief Starts a SunsetSunrise object
/// @return True on success
bool SunsetSunrise::begin() {
	if (!checkConfig(config_path)) {
		// Set defaults 
		SunriseSunset_config.lat = 40.0000;
		SunriseSunset_config.lng = -73.0000;
		SunriseSunset_config.timezone_string = "America/New_York";
		SunriseSunset_config.timezone_offset_seconds = -18000;
		SunriseSunset_config.automatic_timezone_offset = false;
			
		return setConfig(getConfig(), true);
	} else {
		// Load settings
		return setConfig(Storage::readFile(config_path), false);
	}
}

/// @brief Gets the configuration of the SunsetSunrise object
/// @return The configuration as a JSON string
String SunsetSunrise::getConfig() {
	JsonDocument doc;
	doc["lat"] = SunriseSunset_config.lat;
	doc["lng"] = SunriseSunset_config.lng;
	doc["timezone_string"] = SunriseSunset_config.timezone_string;
	doc["timezone_offset_seconds"] = SunriseSunset_config.timezone_offset_seconds;

	String output;
	serializeJson(doc, output);
	return output;
}

/// @brief Sets the configuration for this utility
/// @param config A JSON string of the configuration settings
/// @param save If the configuration should be saved to a file
/// @return True on success
bool SunsetSunrise::setConfig(String config, bool save) {
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, config);
	if (error) {
		Logger.print(F("Deserialization failed: "));
		Logger.println(error.f_str());
		return false;
	}
	SunriseSunset_config.lat = doc["lat"].as<double>();
	SunriseSunset_config.lng = doc["lng"].as<double>();
	SunriseSunset_config.timezone_string = doc["timezone_string"].as<String>();
	SunriseSunset_config.automatic_timezone_offset = doc["automatic_timezone_offset"].as<bool>();
	SunriseSunset_config.timezone_offset_seconds = doc["timezone_offset_seconds"].as<int>();

	// Set sunset position
	if (SunriseSunset_config.automatic_timezone_offset) {
		sunset.setPosition(SunriseSunset_config.lat, SunriseSunset_config.lng, (double)(TimeInterface::getFormattedTime("%z").toInt()/100));
	} else {
		sunset.setPosition(SunriseSunset_config.lat, SunriseSunset_config.lng, (double)(SunriseSunset_config.timezone_offset_seconds/3600));
	}

	if (save) {
		return saveConfig(config_path, config);
	}
	return true;
}

/// @brief Retrieves or calculates the sunset/sunrise time
/// @return True on success
bool SunsetSunrise::getTimes() {
	bool success = false;
	if (SunriseSunset_config.timezone_string != "") {
		Logger.println("Getting sunset/sunrise from API");
		// Allocate the JSON document
		JsonDocument result;
		String getResponse = sunset_hook.getRequest({{"lat", String(SunriseSunset_config.lat)}, {"lng", String(SunriseSunset_config.lng)}, {"formatted", "0"}, {"tzid", SunriseSunset_config.timezone_string}});
		// Deserialize result contents
		DeserializationError error = deserializeJson(result, getResponse);
		// Test if parsing succeeds.
		if (!error && result["code"].as<int>() == 200) {
			JsonDocument response;
			// Deserialize response contents
			DeserializationError error = deserializeJson(response, result["response"].as<String>());
			if (!error && response["status"].as<String>() == "OK") {
				String sunset = response["results"]["sunset"].as<String>();
				String sunrise = response["results"]["sunrise"].as<String>();
				times.sunset_hour = sunset.substring(sunset.indexOf("T")).substring(1,3).toInt();
				times.sunset_minute = sunset.substring(sunset.indexOf("T")).substring(4,6).toInt();
				times.sunrise_hour = sunrise.substring(sunrise.indexOf("T")).substring(1,3).toInt();
				times.sunrise_minute = sunrise.substring(sunrise.indexOf("T")).substring(4,6).toInt();
				success = true;
			} else {
				Logger.println("Sunset API returned error");
				Logger.println(error.f_str());
			}		
		} else {
			Logger.print(F("Error getting sunset: "));
			Logger.println(error.f_str());
			Logger.println(getResponse);
		}
	}
	if (API_fail_count >= 3 || SunriseSunset_config.timezone_string == "") {
		Logger.println("Calculating sunset/sunrise");
		// Keep timezone updated
		if (SunriseSunset_config.automatic_timezone_offset) {
			sunset.setTZOffset((double)(TimeInterface::getFormattedTime("%z").toInt()/100));
		} else {
			sunset.setTZOffset(((double)SunriseSunset_config.timezone_offset_seconds)/3600);
		}
		sunset.setCurrentDate(TimeInterface::getFormattedTime("%Y").toInt(), TimeInterface::getFormattedTime("%m").toInt(), TimeInterface::getFormattedTime("%d").toInt());
		double sunset_raw = sunset.calcSunset();
		double sunrise_raw = sunset.calcSunrise();
		times.sunset_hour = floor(sunset_raw/60);
		times.sunset_minute = std::floor(std::fmod(sunset_raw, 60));
		times.sunrise_hour = std::floor(sunrise_raw/60);
		times.sunrise_minute = std::ceil(std::fmod(sunrise_raw, 60));
		success = true;
	}
	if (success) {
		API_fail_count = 0;
		times.last_updated = TimeInterface::getFormattedTime("%Y-%m-%dT%H:%M:%S");
		return true;
	}
	API_fail_count++;
	return false;
}