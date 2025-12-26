#pragma once

#include <string>
#include <vector>



namespace pubsupp {

	/*
	 * Topic class for the MQTT client.
	 * https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html
	 *
	 * Topic semantic and usage (4.7.3)
	 * - All Topic Names and Topic Filters MUST be at least one character long
	 * - Topic Names and Topic Filters are case sensitive
	 * - Topic Names and Topic Filters can include the space character
	 * - A leading or trailing ‘/’ creates a distinct Topic Name or Topic Filter
	 * - A Topic Name or Topic Filter consisting only of the ‘/’ character is valid
	 * - Topic Names and Topic Filters MUST NOT include the null character (Unicode U+0000)
	 * - Topic Names and Topic Filters are UTF-8 encoded strings, they MUST NOT encode to more than 65535 bytes
	 * - There is no limit to the number of levels in a Topic Name or Topic Filter, other than that imposed by the overall length of a UTF-8 encoded string.
	 */
	class Topic {
		public:
			Topic() = default;
			Topic(const std::string& topic) : topic(topic) {};
			~Topic() = default;

			void set(const std::string& newTopic);
			const std::string& get() { return this->topic; };

			bool isValid();
			bool isValid(const std::string& t);

			bool passesFilter(const std::string& filter);

		private:
			std::string topic;
			std::vector<std::string> topicLevels;

			void parseTopicLevels();
			std::vector<std::string> parseTopicLevels(const std::string& t);
	};
}
