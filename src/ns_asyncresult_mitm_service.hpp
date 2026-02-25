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