#pragma once
#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define DebugSerial	Serial1

inline void PackString(const char* file, const int line) {
	char buff[512];
	int index = 0;
	for (int n = 0; n < strlen(file); n++)
		if (file[n] == '\\' || file[n] == '/')
			index = n + 1;
	sprintf(buff, "[%s:%d] ", file + index, line);
	DebugSerial.print(buff);
}

#define DEBUG_MESSAGE(mesg)	\
	PackString(__FILE__ ,__LINE__); \
	DebugSerial.println(mesg)

#define DEBUG_MESSAGE_TWO(mesg,fix)	\
	PackString(__FILE__ ,__LINE__); \
	DebugSerial.print(mesg); \
	DebugSerial.println(fix)

#else

#define DEBUG_MESSAGE(mesg)
#define DEBUG_MESSAGE_TWO(mesg,fix)

#endif
