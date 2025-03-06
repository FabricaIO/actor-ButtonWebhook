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
#include <Webhook.h>

/// @brief Class describing a button that triggers a webhook
class ButtonWebhook : public Actor, public DigitalInputTrigger {
	protected:
		/// @brief Holds button webhook configuration
		struct {
			/// @brief The request method to use
			String method;
		 } current_config;

		/// @brief Path to configuration file
		String config_path;

		/// @brief Webhook to for sending signal
		Webhook webhook;

	public:
		ButtonWebhook(int Pin, String url, std::map<String, String> customHeaders = {}, String configFile = "ButtonWebhook.json");
		bool begin();
		std::tuple<bool, String> receiveAction(int action, String payload = "");
		String getConfig();
		bool setConfig(String config, bool save);
		JsonDocument addAdditionalConfig();
		void runTask(long elapsed);
};