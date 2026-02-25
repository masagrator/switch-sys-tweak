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
#include "ns_asyncvalue_mitm_service.hpp"
#include "file_utils.hpp"

ams::Result AsyncValueService::IAsyncValue_GetSize(ams::sf::Out<u64> out_size) {
	Result rc = serviceDispatchOut(this->srv.get(), IAsyncValueCmdId::IAsyncValue_GetSize, *out_size);
	FILE_LOG_IPC_CLASS("Handle passed from CMD: %d // %x", m_origin_cmd_id, rc);
	return rc;
}

ams::Result AsyncValueService::IAsyncValue_GetData(const ams::sf::OutMapAliasBuffer &out_buffer) {
	Result rc = serviceDispatch(this->srv.get(), IAsyncValueCmdId::IAsyncValue_GetData,
		.buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
		.buffers = {{ out_buffer.GetPointer(), out_buffer.GetSize() }},
	);
	FILE_LOG_IPC_CLASS("Handle passed from CMD: %d // %x", m_origin_cmd_id, rc);
	return rc;
}

ams::Result AsyncValueService::IAsyncValue_Cancel() {
	Result rc = serviceDispatch(this->srv.get(), IAsyncValueCmdId::IAsyncValue_Cancel);
	FILE_LOG_IPC_CLASS("Handle passed from CMD: %d // %x", m_origin_cmd_id, rc);
	return rc;
}

ams::Result AsyncValueService::IAsyncValue_GetErrorContext(const ams::sf::OutMapAliasBuffer &out_buffer) {
	Result rc = serviceDispatch(this->srv.get(), IAsyncValueCmdId::IAsyncValue_GetErrorContext,
		.buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
		.buffers = {{ out_buffer.GetPointer(), out_buffer.GetSize() }},
	);
	FILE_LOG_IPC_CLASS("Handle passed from CMD: %d // %x", m_origin_cmd_id, rc);
	return rc;
}