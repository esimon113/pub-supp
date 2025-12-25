#include <queue>
#include <memory>


#include "mqttMessage.hpp"



namespace pubsupp {
	// TODO
	class SessionState {
		public:
			SessionState();
			~SessionState();

		private:
			// QoS1 + QoS2 msgs sent to server but not completely acknowledged
			std::queue<std::shared_ptr<MqttMessage>> sentToSrvNotAcked;
			// QoS2 msgs received from server but not completely acknowledged
			std::queue<std::shared_ptr<MqttMessage>> receivedBySrvNotCompleted;
	};



}
