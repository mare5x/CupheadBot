#pragma once
#include "imgui\imgui.h"


// Logger from imgui_demo.cpp
// Usage:
//  my_log.log("Hello %d world\n", 123);
//  my_log.render("title");
struct LoggerUI
{
	ImGuiTextBuffer     Buf;
	ImGuiTextFilter     Filter;
	ImVector<int>       LineOffsets;        // Index to lines offset
	bool                ScrollToBottom;

	void clear();

	void log(const char* fmt, ...);
	void log(const char* fmt, va_list args);

	void render(const char* title, bool* p_open = NULL);
};

// Holds a static logger and wraps its functions.
// Usage:
//	ui_logger::log("Hello %d world\n", 123);
namespace ui_logger {
	extern LoggerUI logger;

	inline void clear() { logger.clear(); }
	inline void log(const char* fmt, ...) { 
		va_list args;
		va_start(args, fmt);
		logger.log(fmt, args); 
		va_end(args);
	}
	inline void error(const char* fmt, ...) {
		logger.log("ERROR: ");
		va_list args;
		va_start(args, fmt);
		logger.log(fmt, args);
		va_end(args);
	}

	inline void render(const char* title, bool* p_open = NULL) { logger.render(title, p_open); }
}