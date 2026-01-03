# MQTT Client Implementation

A lightweight from-scratch MQTT *(Message Queuing Telemetry Transport)* client implementation in C++ as a learning project. This implementation focuses on understanding IoT protocol design, binary message parsing, network programming, and modern C++ practices.


## Motivation

This project was developed to gain hands-on experience with:
- **Protocol Implementation**: Implementing a real-world IoT protocol from specification documents ([MQTT v3.1.1](https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html))
- **Binary Protocol Parsing**: Working with variable-length encoding, bit manipulation, and message serialization/deserialization
- **Network Programming**: Low-level TCP socket programming and asynchronous I/O concepts
- **C++ Best Practices**: Modern C++20 features, RAII, smart pointers, and object-oriented design


## What is MQTT?

MQTT is a lightweight, publish-subscribe messaging protocol designed for IoT and low-bandwidth networks. It enables devices to communicate efficiently by publishing messages to topics and subscribing to receive messages from topics. MQTT is widely used in IoT applications, home automation, and industrial systems.


## Project Description

This implementation includes:

- **Core MQTT Messages**: CONNECT, CONNACK, PUBLISH, PUBACK, SUBSCRIBE, SUBACK, DISCONNECT, UNSUBSCRIBE, UNSUBACK
- **TCP Client Layer**: Custom TCP socket implementation for broker communication
- **Message Parsing**: Binary protocol parsing with variable-length encoding support
- **QoS Support**: Quality of Service levels for message delivery guarantees

**Note**: This is a learning project and is not production-ready. It serves as a demonstration of protocol implementation skills and understanding of network programming fundamentals.


## Building and Running

### Prerequisites
- CMake (3.30 or higher)
- C++20 compatible compiler (GCC/Clang)
- Make


### Build and Run

```bash
./build-run.sh
```

This script will:
1. Clean any existing build directory
2. Create a new build directory
3. Configure the project with CMake
4. Build the executable
5. Run the client

### Minimal Manual Build

```bash
mkdir build && cd build
cmake ../src
make
./pubsupp
```


## Usage

The client connects to a local MQTT broker (default: `127.0.0.1:1883`), subscribes to topics, and can publish messages. Modify `main.cpp` to customize broker address, port, topics, and message payloads.

**Example**: The default implementation subscribes to all topics (`#`) and publishes a test message to the `test` topic.
