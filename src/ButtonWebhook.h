/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2025 Sam Groveman
* 
* External libraries needed:
* ArduinoJSON: https://arduinojson.org/
*
* Contributors: Sam Groveman
*/
#pragma once
#include <Actor.h>
#include <DigitalInputTrigger.h>
#include <WebhookAction.h>

/// @brief Class describing a button that triggers a webhook
class ButtonWebhook : public WebhookAction, public DigitalInputTrigger {
	public:
		ButtonWebhook(String Name, int Pin, String url, std::map<String, String> customHeaders = {}, String configFile = "ButtonWebhook.json");
		bool begin();
		std::tuple<bool, String> receiveAction(int action, String payload = "");
		String getConfig();
		bool setConfig(String config, bool save);
		JsonDocument addAdditionalConfig();
		void runTask(ulong elapsed);

	protected:
		/// @brief Path to configuration file
		String config_path;
};