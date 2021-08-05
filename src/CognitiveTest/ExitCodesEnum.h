#pragma once
enum ExitCodes {
	Successful = 0,

	// Argument error codes
	TooFewArguments = 2,
	TooManyArguments = 3,
	InvalidUserId = 4,
	InvalidSiteName = 5,
	InvalidInterviewerId = 6,
	InvalidLanguage = 7,

	// Setup error codes
	JsonSettingsNotFound = 11,
	CCBExeNotFound = 12,

	TestResultsNotFound = 13,

	// User exit codes
	WrongId = 30,
	UserOptedOutOfTest = 31,
	UserExitedWithoutCompletingTest = 32,

	// Test Codes
	Test = Successful,
	FullTest = 80,

	// Other
	Continue = 98,
	Unkown = 99
};