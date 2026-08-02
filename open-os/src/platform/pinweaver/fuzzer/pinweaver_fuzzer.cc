/* Copyright 2021 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>
#include <utility>

#include "fuzzer_provider.h"
#include "pinweaver.h"

std::unique_ptr<FuzzedDataProvider> g_data_provider;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	g_data_provider = std::make_unique<FuzzedDataProvider>(data, size);
	g_time_base = 0;
	if (g_data_provider->ConsumeBool())
		force_restart_count(
			g_data_provider->ConsumeIntegralInRange<int>(1, 30));
	pinweaver_init();
	while (g_data_provider->remaining_bytes()) {
		int max_sz = sizeof(struct pw_request_t) + PW_MAX_MESSAGE_SIZE;
		size_t length = g_data_provider->ConsumeIntegralInRange<size_t>(
				0, PW_MAX_MESSAGE_SIZE);
		auto request = g_data_provider->ConsumeBytes<char>(max_sz);
		request.resize(max_sz);

		struct pw_request_t *pw_request =
			static_cast<struct pw_request_t *>(
				static_cast<void *>(request.data()));
		pw_request->header.data_length = length;

		size_t request_size = sizeof(pw_request->header) +
				      pw_request->header.data_length;

		std::vector<char> response(max_sz);
		size_t response_size;

		pinweaver_command(request.data(), request_size, response.data(),
				  &response_size);
	}
	return 0;
}
