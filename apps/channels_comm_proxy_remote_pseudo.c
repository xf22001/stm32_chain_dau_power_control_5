

/*================================================================
 *
 *
 *   文件名称：channels_comm_proxy_remote.c
 *   创 建 者：肖飞
 *   创建日期：2021年09月16日 星期四 10时34分46秒
 *   修改日期：2022年06月21日 星期二 11时35分20秒
 *   描    述：
 *
 *================================================================*/
#include "channels_comm_proxy_remote.h"

#include "log.h"

static int init_channels_config_proxy_channel_info(channels_config_t *channels_config)
{
	int ret = 0;
	int i;
	int j;

	channels_config->proxy_channel_info.proxy_channel_number = 0;

	OS_ASSERT(channels_config->proxy_channel_info.items == NULL);

	for(i = 0; i < channels_config->channel_number; i++) {
		channel_config_t *channel_config = channels_config->channel_config[i];

		if(channel_config->channel_type == CHANNEL_TYPE_PROXY_REMOTE) {
			channels_config->proxy_channel_info.proxy_channel_number++;
		}
	}

	if(channels_config->proxy_channel_info.proxy_channel_number == 0) {
		return ret;
	}

	channels_config->proxy_channel_info.items = (proxy_channel_item_t *)os_calloc(channels_config->proxy_channel_info.proxy_channel_number, sizeof(proxy_channel_item_t));
	OS_ASSERT(channels_config->proxy_channel_info.items != NULL);

	j = 0;

	for(i = 0; i < channels_config->channel_number; i++) {
		channel_config_t *channel_config = channels_config->channel_config[i];

		if(channel_config->channel_type == CHANNEL_TYPE_PROXY_REMOTE) {
			proxy_channel_item_t *item = channels_config->proxy_channel_info.items + j;
			item->proxy_channel_index = j;
			item->channel_id = i;

			debug("proxy remote channel %d index %d", item->channel_id, item->proxy_channel_index);

			j++;
		}
	}

	return ret;
}


int start_channels_comm_proxy_remote(channels_info_t *channels_info)
{
	int ret = 0;
	channels_config_t *channels_config = channels_info->channels_config;
	uint8_t proxy_channel_number;
	int i;

	init_channels_config_proxy_channel_info(channels_config);
	proxy_channel_number = channels_config->proxy_channel_info.proxy_channel_number;

	if(proxy_channel_number == 0) {
		return ret;
	}

	for(i = 0; i < channels_info->channel_number; i++) {
		channel_info_t *channel_info = channels_info->channel_info + i;
		proxy_channel_item_t *proxy_channel_item = get_proxy_channel_item_by_channel_id(&channels_config->proxy_channel_info, channel_info->channel_id);

		if(proxy_channel_item == NULL) {
			continue;
		}

		channel_info->connect_state = 10;
	}

	return ret;
}
