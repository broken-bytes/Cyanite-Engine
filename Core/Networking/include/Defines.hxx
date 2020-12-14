#pragma once

// VALUES
constexpr int MAX_PAYLOAD_LENGTH = 1024;
constexpr int TICKRATE_ms = 64; // Ticks per Second
constexpr int SERVER_SLEEP_us = 1000000 / TICKRATE_ms;
constexpr int CONNECT_TIMEOUT_ms = 5000;

// MESSAGE TYPES
constexpr char CONNECT_REQ[] = "CON_REQ_MSG";
constexpr char CONNECT_REJ[] = "CON_REJ_MSG";
constexpr char CONNECT_WARN[] = "CON_WRN_MSG";
constexpr char CONNECT_ACK[] = "CON_ACK_MSG";
constexpr char MSG_ACK[] = "MSG_ACK";

// ERROR SECTION
constexpr int CLIENT_NOT_CREATED = 100;
constexpr int CLIENT_ALREADY_CREATED = 101;
constexpr int CLIENT_ALREADY_CONNECTED = 102;
constexpr int CLIENT_CONNECTION_TIMEOUT = 103;

constexpr int SERVER_NOT_CREATED = 200;
constexpr int SERVER_ALREADY_CREATED = 201;

// MESSAGING
constexpr char MSG_SECTION_DELIMITER[] = "";
// A network package is max 1024 bytes
	// Channel PackageID PREVIOUS_RELIABLE_ID EventData(JSON)
	//    1      10           10            950
constexpr uint16_t CHANNEL_LENGTH = 1;
constexpr uint16_t PackageID_LENGTH = 1;
constexpr uint16_t DATA_LENGTH = 960;