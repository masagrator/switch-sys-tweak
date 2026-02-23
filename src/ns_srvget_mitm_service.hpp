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
    GetSize    = 0,
    GetData    = 1,
    Cancel     = 2,
    GetErrorContext = 3,
};

#define AMS_IASYNCVALUE_INTERFACE_INFO(C, H) \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncValueCmdId, GetSize,         (ams::sf::Out<u64> out_size),                                           (out_size))         \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncValueCmdId, GetData,         (const ams::sf::OutMapAliasBuffer &out_buffer),                         (out_buffer))       \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncValueCmdId, Cancel,          (),                                                                    ())                  \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncValueCmdId, GetErrorContext, (const ams::sf::OutMapAliasBuffer &out_buffer),                         (out_buffer))

AMS_SF_DEFINE_INTERFACE(ams::ns::mitm, IAsyncValue, AMS_IASYNCVALUE_INTERFACE_INFO, 0x8e4d000e)

// Pure virtual base class
class IAsyncValueInterface {
    public:
        virtual ~IAsyncValueInterface() = default;

        // cmd 0: outbytes=8, no buffers -> returns u64 size
        virtual ams::Result GetSize(ams::sf::Out<u64> out_size) = 0;

        // cmd 1: outbytes=0, buffers=[6] (OutMapAlias) -> fills caller's buffer with data
        virtual ams::Result GetData(const ams::sf::OutMapAliasBuffer &out_buffer) = 0;

        // cmd 2: outbytes=0, no buffers -> cancels pending async operation
        virtual ams::Result Cancel() = 0;

        // cmd 3: outbytes=0, buffers=[22] (OutAutoSelect, entry_size=0x200) -> fills error context
        virtual ams::Result GetErrorContext(const ams::sf::OutMapAliasBuffer &out_buffer) = 0;
};

enum IAsyncResultCmdId : u32 {
    Cancel          = 0,
    Wait            = 1,  // or GetResult - blocks/polls completion, no data
    GetErrorContext = 2,
};

#define AMS_IASYNCRESULT_INTERFACE_INFO(C, H) \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncResultCmdId, Cancel,          (),                                            ())           \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncResultCmdId, Wait,            (),                                            ())           \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncResultCmdId, GetErrorContext, (const ams::sf::OutMapAliasBuffer &out_buffer), (out_buffer))

AMS_SF_DEFINE_INTERFACE(ams::ns::mitm, IAsyncResult, AMS_IASYNCRESULT_INTERFACE_INFO, 0x66e1adbd)

class IAsyncResultInterface {
    public:
        virtual ~IAsyncResultInterface() = default;

        // cmd 0: cancel the pending async operation
        virtual ams::Result Cancel() = 0;

        // cmd 1: wait for / poll completion — returns Result indicating success or pending
        virtual ams::Result Wait() = 0;

        // cmd 2: outbytes=0, buffers=[22] entry_size=0x200 -> nn::err::ErrorContext
        virtual ams::Result GetErrorContext(const ams::sf::OutMapAliasBuffer &out_buffer) = 0;
};

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
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData, (u8 flag, u64 tid, const ams::sf::OutBuffer &buffer, ams::sf::Out<u64> out_size), (in_bytes, out_bytes, out_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppDesiredLanguage, (u32 bitmask, ams::sf::Out<u8> out_langentry), (bitmask, out_langentry)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, ConvertAppLanguageToLanguageCode, (u8 langentry, ams::sf::Out<u64> langcode), (langentry, langcode)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, ConvertLanguageCodeToAppLanguage, (u64 langcode, ams::sf::Out<u8> langentry), (langcode, langentry)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, SelectApplicationDesiredLanguage, (sf::Out<Struct0x8> out_bytes, const ams::sf::InMapAliasBuffer &in_buffer), (out_bytes, in_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData5, (Struct0x10 in_bytes, ams::sf::Out<Struct0x8> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer), (in_bytes, out_bytes, out_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData6, (Struct0x10 in_bytes, ams::sf::Out<Struct0x8> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer), (in_bytes, out_bytes, out_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk7, (Struct0x8 in_bytes, ams::sf::Out<Struct0x80> out_bytes), (in_bytes, out_bytes)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk8, (Struct0x88 in_bytes, ams::sf::Out<Struct0x4> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer), (in_bytes, out_bytes, out_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk9, (Struct0x8 in_bytes, const ams::sf::InMapAliasBuffer &in_buffer), (in_bytes, in_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppTitleAsync, (Struct0x10 in_bytes, const ams::sf::InMapAliasArray<Struct0x8> &in_array, ams::sf::CopyHandle in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<sf::SharedPointer<IAsyncValue>> out_interface), (in_bytes, in_buffer, in_handle, out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk11, (Struct0x8 in_bytes, const ams::sf::InMapAliasArray<Struct0x10> &in_array, ams::sf::CopyHandle in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<sf::SharedPointer<IAsyncValue>> out_interface), (in_bytes, in_buffer, in_handle, out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk12, (Struct0x10 in_bytes, const ams::sf::InMapAliasArray<Struct0x10> &in_array, ams::sf::CopyHandle in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<sf::SharedPointer<IAsyncValue>> out_interface), (in_bytes, in_buffer, in_handle, out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk13, (Struct0x8 in_bytes, const ams::sf::InMapAliasArray<Struct0x8> &in_array, ams::sf::CopyHandle in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<sf::SharedPointer<IAsyncValue>> out_interface), (in_bytes, in_buffer, in_handle, out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk14, (Struct0x10 in_bytes, const ams::sf::InMapAliasArray<Struct0x8> &in_array, ams::sf::CopyHandle in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<sf::SharedPointer<IAsyncValue>> out_interface), (in_bytes, in_buffer, in_handle, out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk15, (Struct0x10 in_bytes, const ams::sf::InMapAliasArray<Struct0x8> &in_array, ams::sf::CopyHandle in_handle, ams::sf::OutCopyHandle out_handle, ams::sf::Out<sf::SharedPointer<IAsyncValue>> out_interface), (in_bytes, in_buffer, in_handle, out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk16, (ams::sf::OutCopyHandle out_handle, ams::sf::Out<sf::SharedPointer<IAsyncResult>> out_interface), (out_handle, out_interface)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, Unk17, (Struct0x90 in_bytes, ams::sf::Out<Struct0x4> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer), (in_bytes, out_bytes, out_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData18, (Struct0x10 in_bytes, ams::sf::Out<Struct0x8> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer), (in_bytes, out_bytes, out_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData19, (Struct0x10 in_bytes, ams::sf::Out<Struct0xC> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer), (in_bytes, out_bytes, out_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData20, (Struct0x10 in_bytes, ams::sf::Out<Struct0xC> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer), (in_bytes, out_bytes, out_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData21, (Struct0x10 in_bytes, ams::sf::Out<Struct0xC> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer), (in_bytes, out_bytes, out_buffer)) \
	AMS_SF_METHOD_INFO_F(C, H, NsROAppControlDataInterfaceCmdId, GetAppControlData22, (Struct0x10 in_bytes, ams::sf::Out<Struct0xC> out_bytes, const ams::sf::OutMapAliasBuffer &out_buffer), (in_bytes, out_bytes, out_buffer))

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
