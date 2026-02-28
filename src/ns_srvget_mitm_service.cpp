/*
 * Copyright (c) 2018 p-sam 2026 MasaGratoR
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

#include <switch.h>
#include "ns_srvget_mitm_service.hpp"
#include "file_utils.hpp"


void ini_parse(const char* path, void* buffer, u64 tid) {
	Nacp* nacp = (Nacp*)buffer;

	ams::fs::FileHandle file;
	if (R_FAILED(ams::fs::OpenFile(std::addressof(file), path, ams::fs::OpenMode_Read))) {
		return;
	}
	char ini[1024];
	s64 size;
	ams::fs::GetFileSize(&size, file);
	if (size > 1023) size = 1023;
	ams::fs::ReadFile(file, 0, ini, size);
	ams::fs::CloseFile(file);
	ini[size] = 0;

	if (memcmp(ini, "[override_nacp]", 15)) {
		FileUtils::LogLine("_ProcessControlData(%016lx) // [override_nacp] was not found!", tid);
		return;
	}
	const char* name = strstr(ini, "name=");
	const char* author = strstr(ini, "author=");

	if (name && author) {
		const char* m_name = &name[5];
		const char* m_author = &author[7];
		size_t m_name_length = strcspn(m_name, "\r\n");
		size_t m_author_length = strcspn(m_author, "\r\n");
		if (m_name_length <= 0x200 && m_author_length <= 0x100) { 
			memset((void*)&nacp->lang_data, 0, sizeof(nacp->lang_data));
			for(unsigned int i = 0; i < 16; i++) {
				memcpy(nacp->lang_data.lang[i].name, m_name, m_name_length);
			}
			for(unsigned int i = 0; i < 16; i++) {
				memcpy(nacp->lang_data.lang[i].author, m_author, m_author_length);
			}
			nacp->titles_data_format = 0;
			FileUtils::LogLine("_ProcessControlData(%016lx) // Name and author passed correctly!", tid);
		}
		else {
			FileUtils::LogLine("_ProcessControlData(%016lx) // Size check of name and author failed! name: %d, author: %d", tid, m_name_length, m_author_length);
		}
	}
	else {
		FileUtils::LogLine("_ProcessControlData(%016lx) // Parsing name and author failed! Found name: %d, author: %d", tid, name != nullptr, author != nullptr);
	}

	const char* display_version = strstr(ini, "display_version=");
	if (display_version) {
		const char* m_display_version = &display_version[16];
		size_t m_display_version_length = strcspn(m_display_version, "\r\n");
		if (m_display_version_length <= 0x10) {
			memset(nacp->display_version, 0, sizeof(nacp->display_version));
			memcpy(nacp->display_version, m_display_version, m_display_version_length);
			FileUtils::LogLine("_ProcessControlData(%016lx) // Display version passed correctly!", tid);
		}
		else {
			FileUtils::LogLine("_ProcessControlData(%016lx) // Size check of display version failed!", tid);
		}
	}
	else {
		FileUtils::LogLine("_ProcessControlData(%016lx) // Parsing display version failed!", tid);
	}
}

[[maybe_unused]] static void _ProcessControlData(u64 tid, u8* buf, size_t buf_size, u32* out_size, u8 flag) {
	if(buf_size < sizeof(Nacp)) {
		return;
	}

	char path[0x80] = "";

	ams::util::TSNPrintf(path, sizeof(path), "sdmc:/atmosphere/contents/%016lx/config.ini", tid);
	bool has_file;
	ams::fs::HasFile(&has_file, path);
	if (has_file) ini_parse(path, buf, tid);
	else FileUtils::LogLine("_ProcessControlData(%016lx) // config.ini was not found!", tid);

	void* icon = &buf[sizeof(Nacp)];

	ams::util::TSNPrintf(path, sizeof(path), "sdmc:/atmosphere/contents/%016lx/icon%s.jpg", tid, flag ? "174" : "");
	bool loaded = false;
	ams::fs::HasFile(&has_file, path);
	if (!has_file) {
		FileUtils::LogLine("_ProcessControlData(%016lx) // icon%s.jpg was not found!", tid, flag ? "174" : "");
		return;
	}
	ams::fs::FileHandle file;
	{
		ams::fs::OpenFile(std::addressof(file), path, ams::fs::OpenMode_Read);
		s64 size;
		ams::fs::GetFileSize(&size, file);
		if ((size_t)size <= buf_size - sizeof(Nacp)) {
			ams::fs::ReadFile(file, 0, icon, size);
			*out_size = sizeof(Nacp) + size;
			loaded = true;
		}
		else FileUtils::LogLine("_ProcessControlData(%016lx) %u // JPG too big! File size: %d B, buffer size: %d B", tid, flag, size, buf-size - sizeof(Nacp));
	}
	ON_SCOPE_EXIT { ams::fs::CloseFile(file); };

	FileUtils::LogLine("_ProcessControlData(%016lx) // %u [%ld|%s] %s", tid, flag, *out_size, loaded ? "loaded" : "failed", path);
}

bool NsAm2MitmService::ShouldMitm(const ams::sm::MitmProcessInfo& client_info) {
	bool should_mitm = (client_info.program_id == ams::ncm::SystemAppletId::Qlaunch);
	FILE_LOG_IPC(NSAM2_MITM_SERVICE_NAME, client_info, "() // %s", should_mitm ? "true" : "false");
	return should_mitm;
}

bool NsRoMitmService::ShouldMitm(const ams::sm::MitmProcessInfo& client_info) {
	bool should_mitm = (client_info.program_id == ams::ncm::SystemProgramId::Ppc);
	FILE_LOG_IPC(NSRO_MITM_SERVICE_NAME, client_info, "() // %s", should_mitm ? "true" : "false");
	return should_mitm;
}

ams::Result NsServiceGetterMitmService::GetROAppControlDataInterface(ams::sf::Out<ams::sf::SharedPointer<NsROAppControlDataInterface>> out) {
	Service s;
	Result rc = serviceDispatch(this->m_forward_service.get(), (u32)NsSrvGetterCmdId::GetROAppControlDataInterface,
		.out_num_objects = 1,
		.out_objects = &s,
	);

	if(R_SUCCEEDED(rc)) {
		const ams::sf::cmif::DomainObjectId target_object_id{serviceGetObjectId(&s)};
		out.SetValue(ams::sf::CreateSharedObjectEmplaced<NsROAppControlDataInterface, NsROAppControlDataService>(this->m_client_info, std::make_unique<Service>(s)), target_object_id);
	}

	FILE_LOG_IPC_CLASS("() // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::GetAppControlData(u8 source, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<u32> out_size) {
	const struct {
		u8 source;
		u64 tid;
	} in = {source, tid};

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppControlData, in, *out_size.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{buffer.GetPointer(), buffer.GetSize()}},
	);

	FILE_LOG_IPC_CLASS("(%u, 0x%016lx, buf[0x%lx]) // %x[0x%lx]", source, tid, buffer.GetSize(), rc, out_size.GetValue());

	if(R_SUCCEEDED(rc) && FileUtils::WaitInitialized()) {
		_ProcessControlData(tid, buffer.GetPointer(), buffer.GetSize(), out_size.GetPointer(), 0);
	}
	return rc;
}

ams::Result NsROAppControlDataService::GetAppDesiredLanguage(u32 bitmask, ams::sf::Out<u8> out_langentry) {
	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppDesiredLanguage, bitmask, *out_langentry.GetPointer());
	FILE_LOG_IPC_CLASS("(0x%08x) // %x[%u]", bitmask, rc, out_langentry.GetValue());
	return rc;
}

ams::Result NsROAppControlDataService::ConvertAppLanguageToLanguageCode(u8 langentry, ams::sf::Out<u64> out_langcode) {
	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::ConvertAppLanguageToLanguageCode, langentry, *out_langcode.GetPointer());
	FILE_LOG_IPC_CLASS("(0x%02x) // %x[%u]", langentry, rc, out_langcode.GetValue());
	return rc;
}

ams::Result NsROAppControlDataService::ConvertLanguageCodeToAppLanguage(u64 langcode, ams::sf::Out<u8> out_langentry) {
	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::ConvertLanguageCodeToAppLanguage, langcode, *out_langentry.GetPointer());
	FILE_LOG_IPC_CLASS("(0x%016lx); // %x[0x%02x]", langcode, rc, out_langentry.GetValue());
	return rc;
}

ams::Result NsROAppControlDataService::SelectApplicationDesiredLanguage(ams::sf::Out<u64> out_bytes, const ams::sf::InMapAliasBuffer &in_buffer) {
	Result rc = serviceDispatchOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::SelectApplicationDesiredLanguage, *out_bytes.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_buffer.GetPointer(), in_buffer.GetSize()}},
	);
	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::GetAppControlData5(u8 source, u8 flag, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<u64> out_size) {
	const struct {
		u8 source;
		u8 flag;
		u64 tid;
	} in = {source, flag, tid};

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppControlData5, in, *out_size.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{buffer.GetPointer(), buffer.GetSize()}},
	);

	struct out_data {
		u32 unk;
		u32 size;
	};

	out_data* data = (out_data*)out_size.GetPointer();

	FILE_LOG_IPC_CLASS("(%u, 0x%016lx, %u buf[0x%lx]) // %x[0x%lx]", source, tid, flag, buffer.GetSize(), rc, data->size);

	if(R_SUCCEEDED(rc) && FileUtils::WaitInitialized()) {
		_ProcessControlData(tid, buffer.GetPointer(), buffer.GetSize(), &data->size, flag);
	}
	return rc;
}
ams::Result NsROAppControlDataService::GetAppControlData6(u8 source, u8 flag1, u8 flag2, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<u64> out_size) {
	const struct {
		u8 source;
		u8 flag1;
		u8 flag2;
		u64 tid;
	} in = {source, flag1, flag2, tid};

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppControlData6, in, *out_size.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{buffer.GetPointer(), buffer.GetSize()}},
	);

	struct out_data {
		u32 unk;
		u32 size;
	};

	out_data* data = (out_data*)out_size.GetPointer();

	FILE_LOG_IPC_CLASS("(%u, 0x%016lx, %u %u buf[0x%lx]) // %x[0x%lx]", source, tid, flag1, flag2, buffer.GetSize(), rc, data->size);

	if(R_SUCCEEDED(rc) && FileUtils::WaitInitialized()) {
		//_ProcessControlData(tid, buffer.GetPointer(), buffer.GetSize(), &data->size, flag1);
	}
	return rc;
}

ams::Result NsROAppControlDataService::Unk7(u64 tid, ams::sf::Out<Struct0x80> out_bytes) {

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk7, tid, *out_bytes.GetPointer());

	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk8(Struct0x88 in_bytes, ams::sf::Out<u32> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer) {

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk8, in_bytes, *out_bytes.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{out_buffer.GetPointer(), out_buffer.GetSize()}},
	);

	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk9(u64 tid, const ams::sf::InMapAliasBuffer &in_buffer) {

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk9, tid,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_buffer.GetPointer(), in_buffer.GetSize()}},
	);

	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::GetAppTitleAsync(Struct0x10 in_bytes, const ams::sf::InMapAliasArray<u64> &in_array, ams::sf::CopyHandle&& in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface) {

	struct in_data {
		u8 source;
		u8 reserved[7];
		u64 tmem_size;
	};

	in_data* data = (in_data*)&in_bytes;

	Handle temp_out_handle = INVALID_HANDLE;
	Service temp_out_interface;

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppTitleAsync, in_bytes,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_array.GetPointer(), in_array.GetSize() * sizeof(in_array.GetPointer()[0])}},
        .in_num_handles = 1,
        .in_handles =  { in_handle.GetOsHandle() },
        .out_num_objects = 1,
        .out_objects = &temp_out_interface,
        .out_handle_attrs = { SfOutHandleAttr_HipcCopy },
        .out_handles = { &temp_out_handle },
	);

    if (R_SUCCEEDED(rc)) {
        out_handle.SetValue(temp_out_handle, true);
		const ams::sf::cmif::DomainObjectId target_object_id{serviceGetObjectId(&temp_out_interface)};
		out_interface.SetValue(ams::sf::CreateSharedObjectEmplaced<AsyncValueInterface, AsyncValueService>(this->m_client_info, std::make_unique<Service>(temp_out_interface), NsROAppControlDataInterfaceCmdId::GetAppTitleAsync), target_object_id);
    }

	FILE_LOG_IPC_CLASS("src: %d, tmem_size: 0x%x B, elem: %d // %x", data->source, data->tmem_size, in_array.GetSize(), rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk11(size_t tmem_size, const ams::sf::InMapAliasArray<Struct0x10> &in_array, ams::sf::CopyHandle&& in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface) {

	Handle temp_out_handle = INVALID_HANDLE;
	Service temp_out_interface;

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk11, tmem_size,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_array.GetPointer(), in_array.GetSize() * sizeof(in_array.GetPointer()[0])}},
        .in_num_handles = 1,
        .in_handles =  { in_handle.GetOsHandle() },
        .out_num_objects = 1,
        .out_objects = &temp_out_interface,
        .out_handle_attrs = { SfOutHandleAttr_HipcCopy },
        .out_handles = { &temp_out_handle },
	);

    if (R_SUCCEEDED(rc)) {
        out_handle.SetValue(temp_out_handle, true);
		const ams::sf::cmif::DomainObjectId target_object_id{serviceGetObjectId(&temp_out_interface)};
		out_interface.SetValue(ams::sf::CreateSharedObjectEmplaced<AsyncValueInterface, AsyncValueService>(this->m_client_info, std::make_unique<Service>(temp_out_interface), NsROAppControlDataInterfaceCmdId::Unk11), target_object_id);
    }

	FILE_LOG_IPC_CLASS("tmem_size: 0x%x B, elem: %d // %x", tmem_size, in_array.GetSize(), rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk12(Struct0x10 in_bytes, const ams::sf::InMapAliasArray<Struct0x10> &in_array, ams::sf::CopyHandle&& in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface) {

	Handle temp_out_handle = INVALID_HANDLE;
	Service temp_out_interface;

	struct in_data {
		u8 source;
		u8 reserved[7];
		u64 tmem_size;
	};

	in_data* data = (in_data*)&in_bytes;

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk12, in_bytes,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_array.GetPointer(), in_array.GetSize() * sizeof(in_array.GetPointer()[0])}},
        .in_num_handles = 1,
        .in_handles =  { in_handle.GetOsHandle() },
        .out_num_objects = 1,
        .out_objects = &temp_out_interface,
        .out_handle_attrs = { SfOutHandleAttr_HipcCopy },
        .out_handles = { &temp_out_handle },
	);

    if (R_SUCCEEDED(rc)) {
        out_handle.SetValue(temp_out_handle, true);
		const ams::sf::cmif::DomainObjectId target_object_id{serviceGetObjectId(&temp_out_interface)};
		out_interface.SetValue(ams::sf::CreateSharedObjectEmplaced<AsyncValueInterface, AsyncValueService>(this->m_client_info, std::make_unique<Service>(temp_out_interface), NsROAppControlDataInterfaceCmdId::Unk12), target_object_id);
    }

	FILE_LOG_IPC_CLASS("src: %d, tmem_size: 0x%x B, elem: %d // %x", data->source, data->tmem_size, in_array.GetSize(), rc);
	return rc;
}

ams::Result NsROAppControlDataService::GetAppTitle2Async(size_t tmem_size, const ams::sf::InMapAliasArray<u64> &in_array, ams::sf::CopyHandle&& in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface) {

	Handle temp_out_handle = INVALID_HANDLE;
	Service temp_out_interface;

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppTitle2Async, tmem_size,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_array.GetPointer(), in_array.GetSize() * sizeof(in_array.GetPointer()[0])}},
        .in_num_handles = 1,
        .in_handles =  { in_handle.GetOsHandle() },
        .out_num_objects = 1,
        .out_objects = &temp_out_interface,
        .out_handle_attrs = { SfOutHandleAttr_HipcCopy },
        .out_handles = { &temp_out_handle },
	);

    if (R_SUCCEEDED(rc)) {
        out_handle.SetValue(temp_out_handle, true);
		const ams::sf::cmif::DomainObjectId target_object_id{serviceGetObjectId(&temp_out_interface)};
		out_interface.SetValue(ams::sf::CreateSharedObjectEmplaced<AsyncValueInterface, AsyncValueService>(this->m_client_info, std::make_unique<Service>(temp_out_interface), NsROAppControlDataInterfaceCmdId::GetAppTitle2Async), target_object_id);
    }

	FILE_LOG_IPC_CLASS("tmem_size: 0x%x B, elem: %d // %x", tmem_size, in_array.GetSize(), rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk14(Struct0x10 in_bytes, const ams::sf::InMapAliasArray<u64> &in_array, ams::sf::CopyHandle&& in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface) {

	Handle temp_out_handle = INVALID_HANDLE;
	Service temp_out_interface;

	struct in_data {
		u8 source;
		u8 reserved[7];
		u64 tmem_size;
	};

	in_data* data = (in_data*)&in_bytes;

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk14, in_bytes,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_array.GetPointer(), in_array.GetSize() * sizeof(in_array.GetPointer()[0])}},
        .in_num_handles = 1,
        .in_handles =  { in_handle.GetOsHandle() },
        .out_num_objects = 1,
        .out_objects = &temp_out_interface,
        .out_handle_attrs = { SfOutHandleAttr_HipcCopy },
        .out_handles = { &temp_out_handle },
	);

    if (R_SUCCEEDED(rc)) {
        out_handle.SetValue(temp_out_handle, true);
		const ams::sf::cmif::DomainObjectId target_object_id{serviceGetObjectId(&temp_out_interface)};
		out_interface.SetValue(ams::sf::CreateSharedObjectEmplaced<AsyncValueInterface, AsyncValueService>(this->m_client_info, std::make_unique<Service>(temp_out_interface), NsROAppControlDataInterfaceCmdId::Unk14), target_object_id);
    }

	FILE_LOG_IPC_CLASS("src: %d, tmem_size: 0x%x B, elem: %d // %x", data->source, data->tmem_size, in_array.GetSize(), rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk15(Struct0x10 in_bytes, const ams::sf::InMapAliasArray<u64> &in_array, ams::sf::CopyHandle&& in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface) {

	Handle temp_out_handle = INVALID_HANDLE;
	Service temp_out_interface;

	struct in_data {
		u8 source;
		u8 reserved[7];
		u64 tmem_size;
	};

	in_data* data = (in_data*)&in_bytes;

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk15, in_bytes,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_array.GetPointer(), in_array.GetSize() * sizeof(in_array.GetPointer()[0])}},
        .in_num_handles = 1,
        .in_handles =  { in_handle.GetOsHandle() },
        .out_num_objects = 1,
        .out_objects = &temp_out_interface,
        .out_handle_attrs = { SfOutHandleAttr_HipcCopy },
        .out_handles = { &temp_out_handle },
	);

    if (R_SUCCEEDED(rc)) {
        out_handle.SetValue(temp_out_handle, true);
		const ams::sf::cmif::DomainObjectId target_object_id{serviceGetObjectId(&temp_out_interface)};
		out_interface.SetValue(ams::sf::CreateSharedObjectEmplaced<AsyncValueInterface, AsyncValueService>(this->m_client_info, std::make_unique<Service>(temp_out_interface), NsROAppControlDataInterfaceCmdId::Unk15), target_object_id);
    }

	FILE_LOG_IPC_CLASS("src: %d, tmem_size: 0x%x B, elem: %d // %x", data->source, data->tmem_size, in_array.GetSize(), rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk16(ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncResultInterface>> out_interface) {

	Handle temp_out_handle = INVALID_HANDLE;
	Service temp_out_interface;

	Result rc = serviceDispatch(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk16,
        .out_num_objects = 1,
        .out_objects = &temp_out_interface,
        .out_handle_attrs = { SfOutHandleAttr_HipcCopy },
        .out_handles = { &temp_out_handle },
	);

    if (R_SUCCEEDED(rc)) {
        out_handle.SetValue(temp_out_handle, true);
		const ams::sf::cmif::DomainObjectId target_object_id{serviceGetObjectId(&temp_out_interface)};
		out_interface.SetValue(ams::sf::CreateSharedObjectEmplaced<AsyncResultInterface, AsyncResultService>(this->m_client_info, std::make_unique<Service>(temp_out_interface)), target_object_id);
    }

	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk17(Struct0x90 in_bytes, ams::sf::Out<u32> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer) {

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk8, in_bytes, *out_bytes.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{out_buffer.GetPointer(), out_buffer.GetSize()}},
	);

	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::GetAppControlData18(u8 source, u8 flag1, u8 flag2, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<u64> out_size) {
	const struct {
		u8 source;
		u8 flag1;
		u8 flag2;
		u64 tid;
	} in = {source, flag1, flag2, tid};

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppControlData18, in, *out_size.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{buffer.GetPointer(), buffer.GetSize()}},
	);

	FILE_LOG_IPC_CLASS("(%u, 0x%016lx, buf[0x%lx]) // %x[0x%lx]", source, tid, buffer.GetSize(), rc, out_size.GetValue());

	if(R_SUCCEEDED(rc) && FileUtils::WaitInitialized()) {
		//_ProcessControlData(tid, buffer.GetPointer(), buffer.GetSize(), out_size.GetPointer());
	}
	return rc;
}

ams::Result NsROAppControlDataService::GetAppControlData19(u8 source, u8 flag, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<Struct0xC> out_size) {
	const struct {
		u8 source;
		u8 flag;
		u64 tid;
	} in = {source, flag, tid};

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppControlData19, in, *out_size.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{buffer.GetPointer(), buffer.GetSize()}},
	);

	struct out_data {
		u32 unk1;
		u32 unk2;
		u32 size;
	};

	out_data* data = (out_data*)out_size.GetPointer();

	FILE_LOG_IPC_CLASS("(%u, 0x%016lx, %u buf[0x%lx]) out[0x%x]// %x", source, tid, flag, buffer.GetSize(), data->size, rc);

	if(R_SUCCEEDED(rc) && FileUtils::WaitInitialized()) {
		_ProcessControlData(tid, buffer.GetPointer(), buffer.GetSize(), &data->size, flag);
	}
	return rc;
}

ams::Result NsROAppControlDataService::GetAppControlData20(u8 source, u8 flag1, u8 flag2, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<Struct0xC> out_size) {
	const struct {
		u8 source;
		u8 flag1;
		u8 flag2;
		u64 tid;
	} in = {source, flag1, flag2, tid};

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppControlData20, in, *out_size.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{buffer.GetPointer(), buffer.GetSize()}},
	);

	FILE_LOG_IPC_CLASS("(%u, 0x%016lx, buf[0x%lx]) // %x", source, tid, buffer.GetSize(), rc);

	if(R_SUCCEEDED(rc) && FileUtils::WaitInitialized()) {
		//_ProcessControlData(tid, buffer.GetPointer(), buffer.GetSize(), out_size.GetPointer());
	}
	return rc;
}

ams::Result NsROAppControlDataService::GetAppControlData21(u8 source, u8 flag1, u8 flag2, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<Struct0xC> out_size) {
	const struct {
		u8 source;
		u8 flag1;
		u8 flag2;
		u64 tid;
	} in = {source, flag1, flag2, tid};

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppControlData21, in, *out_size.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{buffer.GetPointer(), buffer.GetSize()}},
	);

	FILE_LOG_IPC_CLASS("(%u, 0x%016lx, buf[0x%lx]) // %x", source, tid, buffer.GetSize(), rc);

	if(R_SUCCEEDED(rc) && FileUtils::WaitInitialized()) {
		//_ProcessControlData(tid, buffer.GetPointer(), buffer.GetSize(), out_size.GetPointer());
	}
	return rc;
}

ams::Result NsROAppControlDataService::GetAppControlData22(u8 source, u8 flag1, u8 flag2, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<Struct0xC> out_size) {
	const struct {
		u8 source;
		u8 flag1;
		u8 flag2;
		u64 tid;
	} in = {source, flag1, flag2, tid};

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppControlData22, in, *out_size.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{buffer.GetPointer(), buffer.GetSize()}},
	);

	FILE_LOG_IPC_CLASS("(%u, 0x%016lx, buf[0x%lx]) // %x", source, tid, buffer.GetSize(), rc);

	if(R_SUCCEEDED(rc) && FileUtils::WaitInitialized()) {
		//_ProcessControlData(tid, buffer.GetPointer(), buffer.GetSize(), out_size.GetPointer());
	}
	return rc;
}