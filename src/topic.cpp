#include "topic.hpp"

#include <iostream>
#include <vector>


namespace pubsupp {
	void Topic::set(const std::string& newTopic) {
		if (this->isValid(newTopic)) {
			this->topic = newTopic;
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
		
		// Handle empty filter
		if (filter.empty()) {
			return topic.empty();
		}

		// Handle '#' as the only character (matches everything)
		if (filter == "#") {
			return true;
		}

		// Validate '#' position - must be at the end and preceded by '/'
		size_t hashPos = filter.find('#');
		if (hashPos != std::string::npos) {
			if (hashPos != filter.length() - 1) {
				return false; // '#' must be at the end
			}
			if (hashPos > 0 && filter[hashPos - 1] != '/') {
				return false; // '#' must be preceded by '/' unless it's the only character
			}
		}

		// Split topic and filter into levels
		std::vector<std::string> topicLevels;
		std::vector<std::string> filterLevels;
		
		size_t start = 0;
		for (size_t i = 0; i <= topic.length(); ++i) {
			if (i == topic.length() || topic[i] == '/') {
				if (i > start) {
					topicLevels.push_back(topic.substr(start, i - start));
				} else {
					topicLevels.push_back(""); // Empty level
				}
				start = i + 1;
			}
		}

		start = 0;
		for (size_t i = 0; i <= filter.length(); ++i) {
			if (i == filter.length() || filter[i] == '/') {
				if (i > start) {
					filterLevels.push_back(filter.substr(start, i - start));
				} else {
					filterLevels.push_back(""); // Empty level
				}
				start = i + 1;
			}
		}

		// Handle '#' at the end of filter
		if (!filterLevels.empty() && filterLevels.back() == "#") {
			filterLevels.pop_back(); // Remove '#' from levels
			// Match all remaining topic levels
			if (filterLevels.size() > topicLevels.size()) {
				return false;
			}
			// Check that all filter levels (except '#') match
			for (size_t i = 0; i < filterLevels.size(); ++i) {
				if (filterLevels[i] == "+") {
					// '+' matches any single level
					continue;
				}
				if (i >= topicLevels.size() || filterLevels[i] != topicLevels[i]) {
					return false;
				}
			}
			return true; // '#' matches all remaining levels
		}

		// No '#' in filter - exact level count match required
		if (filterLevels.size() != topicLevels.size()) {
			return false;
		}

		// Match each level
		for (size_t i = 0; i < filterLevels.size(); ++i) {
			if (filterLevels[i] == "+") {
				// '+' matches any single level
				continue;
			}
			if (filterLevels[i] != topicLevels[i]) {
				return false;
			}
		}

		return true;
	}
}
