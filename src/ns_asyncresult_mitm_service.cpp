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


#include <switch.h>
#include "ns_asyncresult_mitm_service.hpp"
#include "file_utils.hpp"


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