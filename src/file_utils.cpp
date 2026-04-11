/*
 * Copyright (c) 2018 p-sam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "file_utils.hpp"

#define UNUSED(expr) do { (void)(expr); } while (0)

static ams::os::Mutex g_log_mutex{false};
static ams::os::ThreadType g_init_thread;
static std::atomic_bool g_has_initialized = false;
s64 LogOffset;

bool FileUtils::IsInitialized() {
	return g_has_initialized;
}

bool FileUtils::WaitInitialized() {
	if(!g_has_initialized) {
		if(g_init_thread.state == ams::os::ThreadType::State_NotInitialized) {
			return false;
		}
		ams::os::WaitThread(&g_init_thread);
	}
	return true;
}

void FileUtils::LogLine(const char* format, ...) {
#ifdef ENABLE_LOGGING
	std::va_list args;
	va_start(args, format);
	if (g_has_initialized) {
		ams::fs::FileHandle file;
		std::scoped_lock lock(g_log_mutex);
		{
			if (R_FAILED(ams::fs::OpenFile(std::addressof(file), "sdmc:/config/sys-ticon/log.txt", ams::fs::OpenMode_Write | ams::fs::OpenMode_AllowAppend))) {
				return;
			}
			char buffer[1024] = "";
			int len = ams::util::TVSNPrintf(buffer, sizeof(buffer), format, args);
			R_DISCARD(ams::fs::WriteFile(file, LogOffset, buffer, len, ams::fs::WriteOption::Flush));
			LogOffset += len;
			R_DISCARD(ams::fs::WriteFile(file, LogOffset, "\n", 1, ams::fs::WriteOption::Flush));
			LogOffset++;
		}
		ON_SCOPE_EXIT { ams::fs::CloseFile(file); };
	}
	va_end(args);
#else 
	UNUSED(format);
#endif
}

ams::Result FileUtils::Initialize() {
#ifdef ENABLE_LOGGING
	bool has_file;
	ams::fs::FileHandle LogFile;
	R_TRY(ams::fs::HasFile(&has_file, "sdmc:/config/sys-ticon/log.txt"));
	if (!has_file)
	{
		R_TRY(ams::fs::CreateFile("sdmc:/config/sys-ticon/log.txt", 0));
	}
	R_TRY(ams::fs::OpenFile(&LogFile, "sdmc:/config/sys-ticon/log.txt", ams::fs::OpenMode_Write | ams::fs::OpenMode_AllowAppend));
	R_TRY(ams::fs::GetFileSize(&LogOffset, LogFile));
	ams::fs::CloseFile(LogFile);
	g_has_initialized = true;
	FileUtils::LogLine("=== " TARGET " " VERSION " ===");
#else
	g_has_initialized = true;
#endif
	return ams::ResultSuccess();
}

void FileUtils::Exit() {

	if (!g_has_initialized) {
		return;
	}

	g_has_initialized = false;
}
