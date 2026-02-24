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
 
#pragma once

#include "libams.hpp"
#include "ns.h"


struct Struct0x90 {
    u8 data[0x90];
};

struct Struct0x88 {
    u8 data[0x88];
};

struct Struct0x80 {
    u8 data[0x80];
};

struct Struct0x10 {
    u8 data[0x10];
};

struct Struct0xC {
    u8 data[0xC];
};

struct Struct0x8 {
    u8 data[0x8];
};

struct Struct0x4 {
    u8 data[0x4];
};

// Command IDs for IAsyncValue
enum IAsyncValueCmdId : u32 {
    IAsyncValue_GetSize    = 0,
    IAsyncValue_GetData    = 1,
    IAsyncValue_Cancel     = 2,
    IAsyncValue_GetErrorContext = 3,
};

#define AMS_IASYNCVALUE_INTERFACE_INFO(C, H) \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncValueCmdId, IAsyncValue_GetSize,         (ams::sf::Out<u64> out_size),                                           (out_size))         \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncValueCmdId, IAsyncValue_GetData,         (const ams::sf::OutMapAliasBuffer &out_buffer),                         (out_buffer))       \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncValueCmdId, IAsyncValue_Cancel,          (),                                                                    ())                  \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncValueCmdId, IAsyncValue_GetErrorContext, (const ams::sf::OutMapAliasBuffer &out_buffer),                         (out_buffer))

AMS_SF_DEFINE_INTERFACE_F(AsyncValueInterface, AMS_IASYNCVALUE_INTERFACE_INFO, 0x00000002);

class AsyncValueService {
    private:
		ams::sm::MitmProcessInfo m_client_info;
		std::unique_ptr<Service> srv;
    public:
        AsyncValueService(const ams::sm::MitmProcessInfo &cl, std::unique_ptr<Service> s) : m_client_info(cl), srv(std::move(s)) {}

		virtual ~AsyncValueService() {
			serviceClose(srv.get());
		}

		constexpr const char* GetDisplayName() {
			return "AsyncValueService";
		}

		AMS_IASYNCVALUE_INTERFACE_INFO(_, AMS_SF_DECLARE_INTERFACE_METHODS);
};
static_assert(IsAsyncValueInterface<AsyncValueService>);

enum IAsyncResultCmdId : u32 {
    IAsyncResult_Get             = 0,
    IAsyncResult_Cancel          = 1,
    IAsyncResult_GetErrorContext = 2,
};

#define AMS_IASYNCRESULT_INTERFACE_INFO(C, H) \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncResultCmdId, IAsyncResult_Get,             (),                                            ())           \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncResultCmdId, IAsyncResult_Cancel,          (),                                            ())           \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncResultCmdId, IAsyncResult_GetErrorContext, (const ams::sf::OutMapAliasBuffer &out_buffer), (out_buffer))

AMS_SF_DEFINE_INTERFACE_F(AsyncResultInterface, AMS_IASYNCRESULT_INTERFACE_INFO, 0x00000001);

class AsyncResultService {
    private:
		ams::sm::MitmProcessInfo m_client_info;
		std::unique_ptr<Service> srv;
    public:
        AsyncResultService(const ams::sm::MitmProcessInfo &cl, std::unique_ptr<Service> s) : m_client_info(cl), srv(std::move(s)) {}

		virtual ~AsyncResultService() {
			serviceClose(srv.get());
		}

		constexpr const char* GetDisplayName() {
			return "AsyncResultService";
		}

		AMS_IASYNCRESULT_INTERFACE_INFO(_, AMS_SF_DECLARE_INTERFACE_METHODS);
};
static_assert(IsAsyncResultInterface<AsyncResultService>);

enum NsROAppControlDataInterfaceCmdId : u32 {
	GetAppControlData                = 0,
	GetAppDesiredLanguage            = 1,
	ConvertAppLanguageToLanguageCode = 2,
	ConvertLanguageCodeToAppLanguage = 3,
	SelectApplicationDesiredLanguage = 4,
	GetAppControlData5               = 5,
	GetAppControlData6               = 6,
	Unk7                             = 7,
	Unk8                             = 8,
	Unk9                             = 9,
	GetAppTitleAsync                 = 10,
	Unk11                            = 11,
	Unk12                            = 12,
	Unk13                            = 13,
	Unk14                            = 14,
	Unk15                            = 15,
	Unk16                            = 16,
	Unk17                            = 17,
	GetAppControlData18              = 18,
	GetAppControlData19              = 19,
	GetAppControlData20              = 20,
	GetAppControlData21              = 21,
	GetAppControlData22              = 22
};

#define NS_RO_APP_CONTROL_DATA_INTERFACE_INFO(C, H) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData, (u8 flag, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<u32> out_size), (flag, tid, buffer, out_size)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppDesiredLanguage, (u32 bitmask, ams::sf::Out<u8> out_langentry), (bitmask, out_langentry)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, ConvertAppLanguageToLanguageCode, (u8 langentry, ams::sf::Out<u64> langcode), (langentry, langcode)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, ConvertLanguageCodeToAppLanguage, (u64 langcode, ams::sf::Out<u8> langentry), (langcode, langentry)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, SelectApplicationDesiredLanguage, (ams::sf::Out<Struct0x8> out_bytes, const ams::sf::InMapAliasBuffer &in_buffer), (out_bytes, in_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData5, (u8 flag, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<u64> out_size), (flag, tid, buffer, out_size)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData6, (u8 source, u8 flag1, u8 flag2, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<u64> out_size), (source, flag1, flag2, tid, buffer, out_size)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk7, (Struct0x8 in_bytes, ams::sf::Out<Struct0x80> out_bytes), (in_bytes, out_bytes)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk8, (Struct0x88 in_bytes, ams::sf::Out<Struct0x4> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer), (in_bytes, out_bytes, out_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk9, (Struct0x8 in_bytes, const ams::sf::InMapAliasBuffer &in_buffer), (in_bytes, in_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppTitleAsync, (Struct0x10 in_bytes, const ams::sf::InMapAliasArray<Struct0x8> &in_array, ams::sf::CopyHandle &&in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface), (in_bytes, in_array, std::move(in_handle), out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk11, (Struct0x8 in_bytes, const ams::sf::InMapAliasArray<Struct0x10> &in_array, ams::sf::CopyHandle &&in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface), (in_bytes, in_array, std::move(in_handle), out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk12, (Struct0x10 in_bytes, const ams::sf::InMapAliasArray<Struct0x10> &in_array, ams::sf::CopyHandle &&in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface), (in_bytes, in_array, std::move(in_handle), out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk13, (Struct0x8 in_bytes, const ams::sf::InMapAliasArray<Struct0x8> &in_array, ams::sf::CopyHandle &&in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface), (in_bytes, in_array, std::move(in_handle), out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk14, (Struct0x10 in_bytes, const ams::sf::InMapAliasArray<Struct0x8> &in_array, ams::sf::CopyHandle &&in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface), (in_bytes, in_array, std::move(in_handle), out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk15, (Struct0x10 in_bytes, const ams::sf::InMapAliasArray<Struct0x8> &in_array, ams::sf::CopyHandle &&in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncValueInterface>> out_interface), (in_bytes, in_array, std::move(in_handle), out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk16, (ams::sf::OutCopyHandle out_handle, ams::sf::Out<ams::sf::SharedPointer<AsyncResultInterface>> out_interface), (out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk17, (Struct0x90 in_bytes, ams::sf::Out<Struct0x4> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer), (in_bytes, out_bytes, out_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData18, (u8 source, u8 flag1, u8 flag2, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<u64> out_size), (source, flag1, flag2, tid, buffer, out_size)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData19, (u8 source, u8 flag, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<Struct0xC> out_size), (source, flag, tid, buffer, out_size)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData20, (u8 source, u8 flag1, u8 flag2, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<Struct0xC> out_size), (source, flag1, flag2, tid, buffer, out_size)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData21, (u8 source, u8 flag1, u8 flag2, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<Struct0xC> out_size), (source, flag1, flag2, tid, buffer, out_size)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData22, (u8 source, u8 flag1, u8 flag2, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<Struct0xC> out_size), (source, flag1, flag2, tid, buffer, out_size))

AMS_SF_DEFINE_INTERFACE_F(NsROAppControlDataInterface, NS_RO_APP_CONTROL_DATA_INTERFACE_INFO, 0x00000000);

class NsROAppControlDataService {
	private:
		ams::sm::MitmProcessInfo m_client_info;
		std::unique_ptr<Service> srv;
	public:
		NsROAppControlDataService(const ams::sm::MitmProcessInfo &cl, std::unique_ptr<Service> s) : m_client_info(cl), srv(std::move(s)) {}

		virtual ~NsROAppControlDataService() {
			serviceClose(srv.get());
		}

		constexpr const char* GetDisplayName() {
			return "NsROAppControlDataInterface";
		}

		NS_RO_APP_CONTROL_DATA_INTERFACE_INFO(_, AMS_SF_DECLARE_INTERFACE_METHODS);
};
static_assert(IsNsROAppControlDataInterface<NsROAppControlDataService>);

enum class NsSrvGetterCmdId : u32 {
	GetROAppControlDataInterface = 7989,
};

#define NS_SRV_GETTER_MITM_INTERFACE_INFO(C, H) \
	AMS_SF_METHOD_INFO_F(C, H, NsSrvGetterCmdId, GetROAppControlDataInterface, (ams::sf::Out<ams::sf::SharedPointer<NsROAppControlDataInterface>> out), (out))

AMS_SF_DEFINE_MITM_INTERFACE_F(NsServiceGetterMitmInterface, NS_SRV_GETTER_MITM_INTERFACE_INFO, 0xf4ec2d1a);

class NsServiceGetterMitmService : public ams::sf::MitmServiceImplBase {
	public:
		using ::ams::sf::MitmServiceImplBase::MitmServiceImplBase;
		NS_SRV_GETTER_MITM_INTERFACE_INFO(_, AMS_SF_DECLARE_INTERFACE_METHODS);

		constexpr const char* GetDisplayName() {
			return "NsServiceGetterInterface";
		}
};

#define NSAM2_MITM_SERVICE_NAME "ns:am2"

class NsAm2MitmService : public NsServiceGetterMitmService {
	public:
		using ::NsServiceGetterMitmService::NsServiceGetterMitmService;
		static bool ShouldMitm(const ams::sm::MitmProcessInfo& client_info);

		constexpr const char* GetDisplayName() {
			return NSAM2_MITM_SERVICE_NAME;
		}

		static constexpr ams::sm::ServiceName GetServiceName() {
			return ams::sm::ServiceName::Encode(NSAM2_MITM_SERVICE_NAME);
		}

		static constexpr size_t GetMaxSessions() {
			return 4;
		}
};
static_assert(IsNsServiceGetterMitmInterface<NsAm2MitmService>);

#define NSRO_MITM_SERVICE_NAME "ns:ro"

class NsRoMitmService : public NsServiceGetterMitmService {
	public:
		using ::NsServiceGetterMitmService::NsServiceGetterMitmService;
		static bool ShouldMitm(const ams::sm::MitmProcessInfo& client_info);

		constexpr const char* GetDisplayName() {
			return NSRO_MITM_SERVICE_NAME;
		}

		static constexpr ams::sm::ServiceName GetServiceName() {
			return ams::sm::ServiceName::Encode(NSRO_MITM_SERVICE_NAME);
		}

		static constexpr size_t GetMaxSessions() {
			return 4;
		}
};
static_assert(IsNsServiceGetterMitmInterface<NsRoMitmService>);
