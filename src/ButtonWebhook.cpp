#include"ButtonWebhook.h"

/// @brief Creates a button webhook
/// @param Pin Pin to use
/// @param configFile Name of the config file to use
ButtonWebhook::ButtonWebhook(int Pin, String Url, std::map<String, String> customHeaders, String configFile) : DigitalInputTrigger(Pin), webhook(Url, customHeaders) {
	config_path = "/settings/act/" + configFile;
}

/// @brief Starts an output 
/// @return True on success
bool ButtonWebhook::begin() {
	// Set description
	Description.actionQuantity = 1;
	Description.type = "button";
	Description.name = "Button Webhook";
	Description.actions = {{"Fire Webhook", 0}};
	if (DigitalInputTrigger::begin()) {
		// Create settings if necessary
		if (!checkConfig(config_path)) {
			// Set defaults
			current_config.method = "GET";
			digital_config.id = 0;
			digital_config.mode = "INPUT";
			digital_config.taskEnabled = false;
			digital_config.trigger = "NONE";
			task_config.taskName = "ButtonHook";
			task_config.taskPeriod = 1000;
			return setConfig(getConfig(), true);
		} else {
			// Load settings
			return setConfig(Storage::readFile(config_path), false);
		}
	}
	return false;
}

/// @brief Receives an action
/// @param action The action to process (only option is 0 for set output)
/// @param payload A 0 or 1 to set the pin low or high
/// @return JSON response with OK
std::tuple<bool, String> ButtonWebhook::receiveAction(int action, String payload) {
	if (action == 0) {
		runTask(LONG_MAX);
	}	
	return { true, R"({"Response": "OK"})" };
}

/// @brief Gets the current config
/// @return A JSON string of the config
String ButtonWebhook::getConfig() {
	// Allocate the JSON document
	JsonDocument doc = addAdditionalConfig();
	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}

/// @brief Sets the configuration for this device
/// @param config A JSON string of the configuration settings
/// @param save If the configuration should be saved to a file
/// @return True on success
bool ButtonWebhook::setConfig(String config, bool save) {
	if (DigitalInputTrigger::setConfig(config)) {
		// Allocate the JSON document
		JsonDocument doc;
		// Deserialize file contents
		DeserializationError error = deserializeJson(doc, config);
		// Test if parsing succeeds.
		if (error) {
			Logger.print(F("Deserialization failed: "));
			Logger.println(error.f_str());
			return false;
		}
		// Assign loaded values
		webhook.webhook_config.url = doc["url"].as<String>();
		current_config.method = doc["method"]["current"].as<String>();
		
		// Save config if requested
		if (save) {
			return saveConfig(config_path, config);
		}
		return true;
	}
	return false;
}

void ButtonWebhook::runTask(ulong elapsed) {
	if (taskPeriodTriggered(elapsed)) {
		if (triggered) {
			String time = String(lastRunTime + elapsedMillis / 1000);
			String millis = String(elapsedMillis % 1000);
			String payload = "{\"id\":" + String(digital_config.id) + ",\"time\":" + time + ",\"ms\":" + millis + "}";
			Logger.println(payload);
			if (current_config.method == "POST") {
				Logger.println(webhook.postRequest(payload));
			} else if (current_config.method == "GET") {
				Logger.println(webhook.getRequest(payload));
			}				
			clearTrigger();
		}
	}
}

/// @brief Collects all the base class parameters and additional parameters
/// @return a JSON document with all the parameters
JsonDocument ButtonWebhook::addAdditionalConfig() {
	// Allocate the JSON document
  	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, DigitalInputTrigger::getConfig());
	// Test if parsing succeeds.
	if (error) {
		Logger.print(F("Deserialization failed: "));
		Logger.println(error.f_str());
		return doc;
	}
	doc["method"]["current"] = current_config.method;
	doc["method"]["options"][0] = "POST";
	doc["method"]["options"][1] = "GET";
	doc["url"] = webhook.webhook_config.url;

	return doc;
}