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

#include <switch.h>
#include "ns_srvget_mitm_service.hpp"
#include "file_utils.hpp"
#include "ini.h"

static int _ProcessControlDataIniHandler(void* user, const char* section, const char* name, const char* value) {
	Nacp* nacp = (Nacp*)user;

	if (strcasecmp(section, "override_nacp") == 0) {
		if (strcasecmp(name, "name") == 0) {
			for(unsigned int i = 0; i < sizeof(nacp->lang_entries) / sizeof(nacp->lang_entries[0]); i++) {
				strncpy(nacp->lang_entries[i].name, value, sizeof(nacp->lang_entries[i].name)-1);
			}
		} else if (strcasecmp(name, "author") == 0) {
			for(unsigned int i = 0; i < sizeof(nacp->lang_entries) / sizeof(nacp->lang_entries[0]); i++) {
				strncpy(nacp->lang_entries[i].author, value, sizeof(nacp->lang_entries[i].author)-1);
			}
		} else if (strcasecmp(name, "display_version") == 0) {
			strncpy(nacp->display_version, value, sizeof(nacp->display_version)-1);
		} else if (strcasecmp(name, "startup_user_account") == 0) {
			nacp->startup_user_account = (*value == 't' || *value == '1');
		}
	}

	return 1;
}

[[maybe_unused]] static void _ProcessControlData(u64 tid, u8* buf, size_t buf_size, u64* out_size) {
	if(buf_size < sizeof(Nacp)) {
		return;
	}

	char path[50] = {0};

	snprintf(path, sizeof(path)-1, "/atmosphere/contents/%016lx/config.ini", tid);
	ini_parse(path, _ProcessControlDataIniHandler, buf);

	void* icon = &buf[sizeof(Nacp)];
	snprintf(path, sizeof(path)-1, "/atmosphere/contents/%016lx/icon.jpg", tid);
	
	FILE* f = fopen(path, "rb");
	if(f != NULL) {
		fread(icon, buf_size - sizeof(Nacp), 1, f);
		*out_size = sizeof(Nacp) + ftell(f);
		fclose(f);
	}

	FileUtils::LogLine("_ProcessControlData(%016lx) // [%ld|%s] %s", tid, *out_size, f ? "loaded" : "failed", path);
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

ams::Result AsyncResultService::IAsyncResult_Get() {
	return serviceDispatch(this->srv.get(), (u32)IAsyncResultCmdId::IAsyncResult_Get);
}

ams::Result AsyncResultService::IAsyncResult_Cancel() {
	return serviceDispatch(this->srv.get(), (u32)IAsyncResultCmdId::IAsyncResult_Cancel);
}

ams::Result AsyncResultService::IAsyncResult_GetErrorContext(const ams::sf::OutMapAliasBuffer &out_buffer) {
	return serviceDispatch(this->srv.get(), (u32)IAsyncResultCmdId::IAsyncResult_GetErrorContext,
		.buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
		.buffers = {{ out_buffer.GetPointer(), out_buffer.GetSize() }},
	);
}

ams::Result AsyncValueService::IAsyncValue_GetSize(ams::sf::Out<u64> out_size) {
	return serviceDispatchOut(this->srv.get(), IAsyncValueCmdId::IAsyncValue_GetSize, *out_size);
}

ams::Result AsyncValueService::IAsyncValue_GetData(const ams::sf::OutMapAliasBuffer &out_buffer) {
	return serviceDispatch(this->srv.get(), IAsyncValueCmdId::IAsyncValue_GetData,
		.buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
		.buffers = {{ out_buffer.GetPointer(), out_buffer.GetSize() }},
	);
}

ams::Result AsyncValueService::IAsyncValue_Cancel() {
	return serviceDispatch(this->srv.get(), IAsyncValueCmdId::IAsyncValue_Cancel);
}

ams::Result AsyncValueService::IAsyncValue_GetErrorContext(const ams::sf::OutMapAliasBuffer &out_buffer) {
	return serviceDispatch(this->srv.get(), IAsyncValueCmdId::IAsyncValue_GetErrorContext,
		.buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
		.buffers = {{ out_buffer.GetPointer(), out_buffer.GetSize() }},
	);
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

ams::Result NsROAppControlDataService::GetAppControlData(u8 flag, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<u64> out_size) {
	const struct {
		u8 flag;
		u64 tid;
	} in = {flag, tid};

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppControlData, in, *out_size.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{buffer.GetPointer(), buffer.GetSize()}},
	);

	FILE_LOG_IPC_CLASS("(%u, 0x%016lx, buf[0x%lx]) // %x[0x%lx]", flag, tid, buffer.GetSize(), rc, out_size.GetValue());

	if(R_SUCCEEDED(rc) && FileUtils::WaitInitialized()) {
		//_ProcessControlData(tid, buffer.GetPointer(), buffer.GetSize(), out_size.GetPointer());
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

ams::Result NsROAppControlDataService::SelectApplicationDesiredLanguage(ams::sf::Out<Struct0x8> out_bytes, const ams::sf::InMapAliasBuffer &in_buffer) {
	Result rc = serviceDispatchOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::SelectApplicationDesiredLanguage, *out_bytes.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_buffer.GetPointer(), in_buffer.GetSize()}},
	);
	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::GetAppControlData5(u8 flag, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<u64> out_size) {
	const struct {
		u8 flag;
		u64 tid;
	} in = {flag, tid};

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppControlData5, in, *out_size.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{buffer.GetPointer(), buffer.GetSize()}},
	);

	FILE_LOG_IPC_CLASS("(%u, 0x%016lx, buf[0x%lx]) // %x[0x%lx]", flag, tid, buffer.GetSize(), rc, out_size.GetValue());

	if(R_SUCCEEDED(rc) && FileUtils::WaitInitialized()) {
		//_ProcessControlData(tid, buffer.GetPointer(), buffer.GetSize(), out_size.GetPointer());
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

	FILE_LOG_IPC_CLASS("(%u, 0x%016lx, buf[0x%lx]) // %x[0x%lx]", source, tid, buffer.GetSize(), rc, out_size.GetValue());

	if(R_SUCCEEDED(rc) && FileUtils::WaitInitialized()) {
		//_ProcessControlData(tid, buffer.GetPointer(), buffer.GetSize(), out_size.GetPointer());
	}
	return rc;
}

ams::Result NsROAppControlDataService::Unk7(Struct0x8 in_bytes, ams::sf::Out<Struct0x80> out_bytes) {

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk7, in_bytes, *out_bytes.GetPointer());

	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk8(Struct0x88 in_bytes, ams::sf::Out<Struct0x4> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer) {

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk8, in_bytes, *out_bytes.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{out_buffer.GetPointer(), out_buffer.GetSize()}},
	);

	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk9(Struct0x8 in_bytes, const ams::sf::InMapAliasBuffer &in_buffer) {

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk9, in_bytes,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_buffer.GetPointer(), in_buffer.GetSize()}},
	);

	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::GetAppTitleAsync(Struct0x10 in_bytes, const ams::sf::InMapAliasArray<Struct0x8> &in_array, ams::sf::CopyHandle&& in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface) {

	Handle temp_out_handle = INVALID_HANDLE;
	Service temp_out_interface;

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppTitleAsync, in_bytes,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_array.GetPointer(), in_array.GetSize()}},
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
		out_interface.SetValue(ams::sf::CreateSharedObjectEmplaced<AsyncValueInterface, AsyncValueService>(this->m_client_info, std::make_unique<Service>(temp_out_interface)), target_object_id);
    }

	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk11(Struct0x8 in_bytes, const ams::sf::InMapAliasArray<Struct0x10> &in_array, ams::sf::CopyHandle&& in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface) {

	Handle temp_out_handle = INVALID_HANDLE;
	Service temp_out_interface;

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk11, in_bytes,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_array.GetPointer(), in_array.GetSize()}},
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
		out_interface.SetValue(ams::sf::CreateSharedObjectEmplaced<AsyncValueInterface, AsyncValueService>(this->m_client_info, std::make_unique<Service>(temp_out_interface)), target_object_id);
    }

	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk12(Struct0x10 in_bytes, const ams::sf::InMapAliasArray<Struct0x10> &in_array, ams::sf::CopyHandle&& in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface) {

	Handle temp_out_handle = INVALID_HANDLE;
	Service temp_out_interface;

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk12, in_bytes,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_array.GetPointer(), in_array.GetSize()}},
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
		out_interface.SetValue(ams::sf::CreateSharedObjectEmplaced<AsyncValueInterface, AsyncValueService>(this->m_client_info, std::make_unique<Service>(temp_out_interface)), target_object_id);
    }

	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk13(Struct0x8 in_bytes, const ams::sf::InMapAliasArray<Struct0x8> &in_array, ams::sf::CopyHandle&& in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface) {

	Handle temp_out_handle = INVALID_HANDLE;
	Service temp_out_interface;

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk13, in_bytes,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_array.GetPointer(), in_array.GetSize()}},
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
		out_interface.SetValue(ams::sf::CreateSharedObjectEmplaced<AsyncValueInterface, AsyncValueService>(this->m_client_info, std::make_unique<Service>(temp_out_interface)), target_object_id);
    }

	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk14(Struct0x10 in_bytes, const ams::sf::InMapAliasArray<Struct0x8> &in_array, ams::sf::CopyHandle&& in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface) {

	Handle temp_out_handle = INVALID_HANDLE;
	Service temp_out_interface;

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk14, in_bytes,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_array.GetPointer(), in_array.GetSize()}},
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
		out_interface.SetValue(ams::sf::CreateSharedObjectEmplaced<AsyncValueInterface, AsyncValueService>(this->m_client_info, std::make_unique<Service>(temp_out_interface)), target_object_id);
    }

	FILE_LOG_IPC_CLASS("(); // %x", rc);
	return rc;
}

ams::Result NsROAppControlDataService::Unk15(Struct0x10 in_bytes, const ams::sf::InMapAliasArray<Struct0x8> &in_array, ams::sf::CopyHandle&& in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface) {

	Handle temp_out_handle = INVALID_HANDLE;
	Service temp_out_interface;

	Result rc = serviceDispatchIn(this->srv.get(), NsROAppControlDataInterfaceCmdId::Unk15, in_bytes,
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
		.buffers = {{in_array.GetPointer(), in_array.GetSize()}},
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
		out_interface.SetValue(ams::sf::CreateSharedObjectEmplaced<AsyncValueInterface, AsyncValueService>(this->m_client_info, std::make_unique<Service>(temp_out_interface)), target_object_id);
    }

	FILE_LOG_IPC_CLASS("(); // %x", rc);
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

ams::Result NsROAppControlDataService::Unk17(Struct0x90 in_bytes, ams::sf::Out<Struct0x4> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer) {

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

ams::Result NsROAppControlDataService::GetAppControlData19(u8 source, u8 flag1, u8 flag2, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<Struct0xC> out_size) {
	const struct {
		u8 source;
		u8 flag1;
		u8 flag2;
		u64 tid;
	} in = {source, flag1, flag2, tid};

	Result rc = serviceDispatchInOut(this->srv.get(), NsROAppControlDataInterfaceCmdId::GetAppControlData19, in, *out_size.GetPointer(),
		.buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
		.buffers = {{buffer.GetPointer(), buffer.GetSize()}},
	);

	FILE_LOG_IPC_CLASS("(%u, 0x%016lx, buf[0x%lx]) // %x", source, tid, buffer.GetSize(), rc);

	if(R_SUCCEEDED(rc) && FileUtils::WaitInitialized()) {
		//_ProcessControlData(tid, buffer.GetPointer(), buffer.GetSize(), out_size.GetPointer());
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