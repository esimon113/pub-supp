#include "topic.hpp"

#include <iostream>
#include <vector>
#include <algorithm>


namespace pubsupp {
	void Topic::set(const std::string& newTopic) {
		if (this->isValid(newTopic)) {
			this->topic = newTopic;
			this->parseTopicLevels();
		} else std::cerr << "Invalid topic: " << newTopic << std::endl;
	}


	bool Topic::isValid() {
		return isValid(this->topic);
	}


	bool Topic::isValid(const std::string& t) {
		if (t.empty()) return false;
		if (t.length() > 65535) return false;
		if (t.find('\0') != std::string::npos) return false;
		if (t.find('#') != std::string::npos || t.find('+') != std::string::npos) return false;

		return true;
	}


	void Topic::parseTopicLevels() {
		this->topicLevels.reserve(std::count(this->topic.begin(), this->topic.end(), '/') + 1);

		size_t start = 0;
		while (true) {
			size_t pos = this->topic.find('/', start);

			if (pos == std::string::npos) { // last level
				this->topicLevels.emplace_back(this->topic.substr(start));
				break;
			}
			this->topicLevels.emplace_back(this->topic.substr(start, pos - start));
			start  = pos + 1;
		}
	}


	/*
	 * Check if a topic passes a filter.
	 *
	 * Topic filter matching rules (4.7)
	 * - '+' matches exactly one topic level
	 * - '#' matches zero or more topic levels and must be at the end
	 * - '#' must be preceded by '/' unless it's the only character
	 */
	bool Topic::passesFilter(const std::string& filter) {
		const std::string& topic = this->topic;

		if (filter.empty()) return topic.empty();
		if (filter == "#") return true;

		// Validate '#' position - must be at the end and preceded by '/'
		size_t hashPos = filter.find('#');
		if (hashPos != std::string::npos) {
			if (hashPos != filter.length() - 1) return false;
			if (hashPos > 0 && filter[hashPos - 1] != '/') return false;
		}

		this->parseTopicLevels();
		std::vector<std::string> filterLevels = this->parseTopicLevels(filter);

		// Handle '#' at the end of filter
		if (!filterLevels.empty() && filterLevels.back() == "#") {
			filterLevels.pop_back(); // Remove '#' from levels
			// Match all remaining topic levels
			if (filterLevels.size() > topicLevels.size()) return false;

			// Check that all filter levels (except '#') match
			for (size_t i = 0; i < filterLevels.size(); ++i) {
				if (filterLevels[i] == "+") continue;
				if (i >= topicLevels.size() || filterLevels[i] != topicLevels[i]) return false;
			}
			return true; // '#' matches all remaining levels
		}

		// No '#' in filter - exact level count match required
		if (filterLevels.size() != topicLevels.size()) return false;

		// Match each level
		for (size_t i = 0; i < filterLevels.size(); ++i) {
			if (filterLevels[i] == "+") continue;
			if (filterLevels[i] != topicLevels[i]) return false;
		}

		return true;
	}
}
