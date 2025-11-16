//================================================================================================
/// @file main.cpp
///
/// @brief Defines `main` for the seeder example
/// @details This example is meant to use all the major protocols in a more "complete" application.
/// @author Adrian Del Grosso
///
/// @copyright 2023 The Open-Agriculture Developers
//================================================================================================

#include <esp_log.h>

#include "esp_littlefs.h"
#include "seeder.hpp"

static const char TAG[] = "AgIsoStack";

// littlefs
#define IOP_PARTITION_PATH "/iop"
#define IOP_PARTITION_LABEL "iop"

esp_err_t Init_littlefs(void)
{
	ESP_LOGI(TAG, "Initializing littlefs");

	if (esp_littlefs_mounted(IOP_PARTITION_LABEL))
	{
		ESP_LOGW(TAG, "littlefs Already Mounted!");
		return ESP_OK;
	}

	esp_vfs_littlefs_conf_t conf = {
		.base_path = IOP_PARTITION_PATH,
		.partition_label = IOP_PARTITION_LABEL,
		.format_if_mount_failed = false,
	};

	esp_err_t ret = esp_vfs_littlefs_register(&conf);
	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		}
		else if (ret == ESP_ERR_NO_MEM)
		{
			ESP_LOGE(TAG, "objects could not be allocated");
		}
		else if (ret == ESP_ERR_INVALID_STATE)
		{
			ESP_LOGE(TAG, "already mounted or partition is encrypted");
		}
		else if (ret == ESP_ERR_NOT_FOUND)
		{
			ESP_LOGE(TAG, "Failed to find littlefs partition");
		}
		else
		{
			ESP_LOGE(TAG, "Failed to initialize littlefs (%s)", esp_err_to_name(ret));
		}
		return ESP_FAIL;
	}
	else
	{
		ESP_LOGI(TAG, "Filesystem mounted");
	}

	size_t total = 0, used = 0;

	ret = esp_littlefs_info(IOP_PARTITION_LABEL, &total, &used);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to get littlefs partition information (%s)", esp_err_to_name(ret));
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);

	return ESP_OK;
}

extern "C" void app_main()
{
	// Init littlefs
	// ********************************************
	if (Init_littlefs() != ESP_OK)
	{
		ESP_LOGE(TAG, "!!! Device init aborted at step: Init_littlefs() !!!");
		return; // No way to continue without working littlefs!
	}

	Seeder seederExample;
	if (seederExample.initialize())
	{
		while (1)
		{
			seederExample.update();
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		seederExample.terminate();
	}
}
