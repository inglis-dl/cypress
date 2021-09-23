#pragma once
enum class ResponseTypeEnum {
	Ack = 6,
	Nack = 21,
	Buttons = 85,
	Notification = 33,

	//Data
	BpAverage = 65,
	BpResult = 66,
	DeflatingCuffPressure = 68,
	InflatingCuffPressure = 73,
	Review = 76
};