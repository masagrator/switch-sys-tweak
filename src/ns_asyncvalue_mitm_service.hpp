/*
 * Copyright (c) 2026 MasaGratoR
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
    AMS_SF_METHOD_INFO_F(C, H, IAsyncValueCmdId, IAsyncValue_Cancel,          (),                                                                     ())                  \
    AMS_SF_METHOD_INFO_F(C, H, IAsyncValueCmdId, IAsyncValue_GetErrorContext, (const ams::sf::OutMapAliasBuffer &out_buffer),                         (out_buffer))

AMS_SF_DEFINE_INTERFACE_F(AsyncValueInterface, AMS_IASYNCVALUE_INTERFACE_INFO, 0x00000002);

class AsyncValueService {
    private:
		ams::sm::MitmProcessInfo m_client_info;
		std::unique_ptr<Service> srv;
		u32 m_origin_cmd_id;
    public:
        AsyncValueService(const ams::sm::MitmProcessInfo &cl, std::unique_ptr<Service> s, u32 origin_cmd_id) : m_client_info(cl), srv(std::move(s)), m_origin_cmd_id(origin_cmd_id) {}

		virtual ~AsyncValueService() {
			serviceClose(srv.get());
		}

		constexpr const char* GetDisplayName() {
			return "AsyncValueService";
		}

		u32 GetOriginCmdId() const {
			return m_origin_cmd_id;
		}

		AMS_IASYNCVALUE_INTERFACE_INFO(_, AMS_SF_DECLARE_INTERFACE_METHODS);
};
static_assert(IsAsyncValueInterface<AsyncValueService>);