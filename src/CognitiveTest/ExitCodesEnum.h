#pragma once
enum ExitCodes {
	Successful = 0,

	// Input error codes (2)
	UnknownMode = 2,
	ProblemWithJsonInputsPath = 2,
	InvalidOrMissingUserId = 2,
	InvalidOrMissingSitename = 2,
	InvalidOrMissingInterviewerID = 2,
	InvalidOrMissingLanguage = 2,

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