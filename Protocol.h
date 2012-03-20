// Defines standards for the RuG Kinect Interface Communication Protocol

#pragma once

// When you first connect to the Server, add this message
#define KINECT_CONNECTION_REQUEST_MESSAGE "Request connection"

// and the type of connection you want to make (you have to send both at once)
enum KINECT_INTERFACE_CONNECTION_TYPE {
	KINECT_INTERFACE_CONNECTION_BASIC
};

// When the server has set up the connection, you'll receive
#define KINECT_CONNECTION_CONFIRMATION_MESSAGE "Connection confirmed"
