#pragma once

#include <cstdio>
#include <varargs.h>

namespace hf
{
	class Log
	{
	public:

		static void Info(const char* fmt, ...)
		{
			const char* open = "\u001b[37m[\u001b[32mInfo\u001b[37m] ";
			printf(open);
			va_list args;
			va_start(args, fmt);
			vprintf(fmt, args);

			va_end(args);
			printf("\n");

			//printf("\u001b[37m[\u001b[32mInfo\u001b[37m] %s\n", fmt);
		}

		static void Warn(const char* fmt, ...)
		{
			const char* open = "\u001b[37m[\u001b[33mWarning\u001b[37m] ";
			printf(open);
			va_list args;
			va_start(args, fmt);
			vprintf(fmt, args);

			va_end(args);
			printf("\n");

			//printf("\u001b[37m[\u001b[33mWarning\u001b[37m] %s\n", fmt);
		}

		static void Error(const char* fmt, ...)
		{
			const char* open = "\u001b[37m[\u001b[31mError\u001b[37m] ";
			printf(open);
			va_list args;
			va_start(args, fmt);
			vprintf(fmt, args);

			va_end(args);
			printf("\n");

			//printf("\u001b[37m[\u001b[31mError\u001b[37m] %s\n", fmt);
		}

		// Fatal 
		static void Fatal(const char* fmt, ...)
		{
			const char* open = "\u001b[37m[\u001b[31mFatal Error\u001b[37m] ";
			printf(open);
			va_list args;
			va_start(args, fmt);
			vprintf(fmt, args);

			va_end(args);
			printf("\n");

			//printf("\u001b[37m[\u001b[31mFatal Error\u001b[37m] %s\n", fmt);

		}

	private:
	};
}