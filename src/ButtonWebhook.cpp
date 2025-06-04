#include"ButtonWebhook.h"

/// @brief Creates a button webhook
/// @param Name The device name
/// @param Pin Pin to use
/// @param Url The URL to call
/// @param customHeaders a <String, String> map of custom headers
/// @param configFile Name of the config file to use
ButtonWebhook::ButtonWebhook(String Name, int Pin, String Url, std::map<String, String> customHeaders, String configFile) : DigitalInputTrigger(Pin), WebhookAction(Name, Url, customHeaders, configFile) {
	config_path = "/settings/act/" + configFile;
}

/// @brief Starts an webhook button 
/// @return True on success
bool ButtonWebhook::begin() {
	// Set description
	Description.actionQuantity = 1;
	Description.type = "button";
	Description.actions = {{"Fire Webhook", 0}};
	if (DigitalInputTrigger::begin()) {
			if (WebhookAction::begin()) {
			// Create settings if necessary
			if (!checkConfig(config_path)) {
				// Set defaults
				hook_config.method = "GET";
				digital_config.id = 0;
				digital_config.mode = "INPUT";
				digital_config.taskEnabled = false;
				digital_config.trigger = "NONE";
				task_config.set_taskName(Description.name.c_str());
				task_config.taskPeriod = 1000;
				return setConfig(getConfig(), true);
			} else {
				// Load settings
				return setConfig(Storage::readFile(config_path), false);
			}
		}
	}
	return false;
}

/// @brief Receives an action
/// @param action The action to process (only option is 0 fire webhook)
/// @param payload Not used
/// @return JSON response with OK
std::tuple<bool, String> ButtonWebhook::receiveAction(int action, String payload) {
	if (action == 0) {
		runTask(LONG_MAX);
	}	
	return { true, R"({"success": true, "Response": "OK"})" };
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
		if (WebhookAction::setConfig(config, false)) {			
			// Save config if requested
			if (save) {
				return saveConfig(config_path, config);
			}
			return true;
		}
	}
	return false;
}

/// @brief Runs the webhook task
/// @param elapsed The milliseconds since this function was last called
void ButtonWebhook::runTask(ulong elapsed) {
	if (taskPeriodTriggered(elapsed)) {
		ulong cached_interrupt_time;
		ulong cached_current_millis;
		ulong cached_last_runtime;
		bool is_triggered;
		portENTER_CRITICAL(&spinlock);
		is_triggered = triggered;
		if (is_triggered) {
			cached_interrupt_time = interrupt_time;
			cached_current_millis = currentMillis;
			cached_last_runtime = lastRunTime;
		}
		portEXIT_CRITICAL(&spinlock);
		if (is_triggered) {
			uint64_t interrupt_millis = cached_interrupt_time / 1000ULL;
			elapsedMillis = (interrupt_millis >= cached_current_millis) ? (interrupt_millis - cached_current_millis) : (ULONG_MAX - cached_current_millis + interrupt_millis);
			ulong time = cached_last_runtime + elapsedMillis / 1000;
			ulong millis = elapsedMillis % 1000;

			// Allocate JSON document
			JsonDocument doc;
            doc["id"] = digital_config.id;
            doc["time"] = time;
            doc["ms"] = millis;
			String payload;

            serializeJson(doc, payload);

			Logger.println(payload);
			Logger.println(WebhookAction::callHook(payload));
			clearTrigger();
		}
	}
}

/// @brief Collects all the base class parameters and additional parameters
/// @return a JSON document with all the parameters
JsonDocument ButtonWebhook::addAdditionalConfig() {
	// Allocate the JSON document
  	JsonDocument doc;
	// Deserialize settings contents
	DeserializationError error = deserializeJson(doc, DigitalInputTrigger::getConfig());
	// Test if parsing succeeds.
	if (error) {
		Logger.print(F("Deserialization failed: "));
		Logger.println(error.f_str());
		return doc;
	}
	// Allocate second JSON document
	JsonDocument doc2;
	// Deserialize settings contents
	error = deserializeJson(doc2, WebhookAction::getConfig());
	// Test if parsing succeeds.
	if (error) {
		Logger.print(F("Deserialization failed: "));
		Logger.println(error.f_str());
		return doc;
	}

	// Merge documents
	for (JsonPairConst kvp : doc2.as<JsonObjectConst>())
	{
		doc[kvp.key()] = kvp.value();
	}

	return doc;
}