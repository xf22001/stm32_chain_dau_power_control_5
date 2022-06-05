

/*================================================================
 *
 *
 *   文件名称：power_manager_group_policy_handler.c
 *   创 建 者：肖飞
 *   创建日期：2022年06月02日 星期四 16时27分25秒
 *   修改日期：2022年06月05日 星期日 14时04分59秒
 *   描    述：
 *
 *================================================================*/
#include "power_manager.h"

#include "relay_boards_comm_proxy_remote.h"
#include "channels_comm_proxy.h"
#include "main.h"

#include "log.h"

extern ADC_HandleTypeDef hadc1;

typedef struct {
	uint8_t relay_id;
	uint8_t channel_id[2];
	GPIO_TypeDef *gpio_port;
	uint16_t gpio_pin;
	GPIO_TypeDef *gpio_port_fb;
	uint16_t gpio_pin_fb;
} relay_node_info_t;

typedef struct {
	uint8_t power_manager_group_id;
	uint8_t size;
	relay_node_info_t **relay_node_info;
} power_manager_group_relay_info_t;

typedef struct {
	uint8_t size;
	power_manager_group_relay_info_t **power_manager_group_relay_info;
} relay_info_t;

static relay_node_info_t relay_node_info_0_0 = {
	.relay_id = 0,
	.channel_id = {0, 1},
	.gpio_port = kg1_GPIO_Port,//km1-kg1
	.gpio_pin = kg1_Pin,
	.gpio_port_fb = fb1_GPIO_Port,
	.gpio_pin_fb = fb1_Pin,
};

static relay_node_info_t relay_node_info_0_1 = {
	.relay_id = 1,
	.channel_id = {1, 2},
	.gpio_port = kg2_GPIO_Port,//km2-kg2
	.gpio_pin = kg2_Pin,
	.gpio_port_fb = fb2_GPIO_Port,
	.gpio_pin_fb = fb2_Pin,
};

static relay_node_info_t relay_node_info_0_2 = {
	.relay_id = 2,
	.channel_id = {2, 3},
	.gpio_port = kg3_GPIO_Port,//km3-kg3
	.gpio_pin = kg3_Pin,
	.gpio_port_fb = fb3_GPIO_Port,
	.gpio_pin_fb = fb3_Pin,
};

static relay_node_info_t relay_node_info_0_3 = {
	.relay_id = 3,
	.channel_id = {3, 4},
	.gpio_port = kg4_GPIO_Port,//km4-kg4
	.gpio_pin = kg4_Pin,
	.gpio_port_fb = fb4_GPIO_Port,
	.gpio_pin_fb = fb4_Pin,
};

static relay_node_info_t relay_node_info_0_4 = {
	.relay_id = 4,
	.channel_id = {4, 0},
	.gpio_port = kg5_GPIO_Port,//km5-kg5
	.gpio_pin = kg5_Pin,
	.gpio_port_fb = fb5_GPIO_Port,
	.gpio_pin_fb = fb5_Pin,
};

static relay_node_info_t *relay_node_info_0_sz[] = {
	&relay_node_info_0_0,
	&relay_node_info_0_1,
	&relay_node_info_0_2,
	&relay_node_info_0_3,
	&relay_node_info_0_4,
};

static power_manager_group_relay_info_t power_manager_group_relay_info_0 = {
	.power_manager_group_id = 0,
	.relay_node_info = relay_node_info_0_sz,
	.size = ARRAY_SIZE(relay_node_info_0_sz),
};

static power_manager_group_relay_info_t *power_manager_relay_info_sz[] = {
	&power_manager_group_relay_info_0,
};

static relay_info_t relay_info = {
	.size = ARRAY_SIZE(power_manager_relay_info_sz),
	.power_manager_group_relay_info = power_manager_relay_info_sz,
};

static relay_node_info_t *get_relay_node_info_by_channel_id(uint8_t power_manager_group_id, uint8_t channel_id_a, uint8_t channel_id_b)
{
	relay_node_info_t *relay_node_info = NULL;
	power_manager_group_relay_info_t *power_manager_group_relay_info = NULL;
	int i;

	OS_ASSERT(channel_id_a != channel_id_b);

	for(i = 0; i < relay_info.size; i++) {
		power_manager_group_relay_info_t *power_manager_group_relay_info_item = relay_info.power_manager_group_relay_info[i];

		if(power_manager_group_relay_info_item->power_manager_group_id == power_manager_group_id) {
			power_manager_group_relay_info = power_manager_group_relay_info_item;
			break;
		}
	}

	if(power_manager_group_relay_info == NULL) {
		return relay_node_info;
	}

	for(i = 0; i < power_manager_group_relay_info->size; i++) {
		relay_node_info_t *relay_node_info_item = power_manager_group_relay_info->relay_node_info[i];
		int j;
		uint8_t found = 1;

		for(j = 0; j < 2; j++) {
			if(channel_id_a == relay_node_info_item->channel_id[j]) {
				break;
			}
		}

		if(j == 2) {
			found = 0;
			continue;
		}

		for(j = 0; j < 2; j++) {
			if(channel_id_b == relay_node_info_item->channel_id[j]) {
				break;
			}
		}

		if(j == 2) {
			found = 0;
			continue;
		}

		if(found != 0) {
			relay_node_info = relay_node_info_item;
			break;
		}
	}

	return relay_node_info;
}

static relay_node_info_t *get_relay_node_info_by_relay_id(uint8_t power_manager_group_id, uint8_t relay_id)
{
	relay_node_info_t *relay_node_info = NULL;
	power_manager_group_relay_info_t *power_manager_group_relay_info = NULL;
	int i;

	for(i = 0; i < relay_info.size; i++) {
		power_manager_group_relay_info_t *power_manager_group_relay_info_item = relay_info.power_manager_group_relay_info[i];

		if(power_manager_group_relay_info_item->power_manager_group_id == power_manager_group_id) {
			power_manager_group_relay_info = power_manager_group_relay_info_item;
			break;
		}
	}

	if(power_manager_group_relay_info == NULL) {
		return relay_node_info;
	}

	for(i = 0; i < power_manager_group_relay_info->size; i++) {
		relay_node_info_t *relay_node_info_item = power_manager_group_relay_info->relay_node_info[i];

		if(relay_id == relay_node_info_item->relay_id) {
			relay_node_info = relay_node_info_item;
			break;
		}
	}

	return relay_node_info;
}

//一个通道绑定的电源模块组
typedef struct {
	uint8_t channel_id;
	uint8_t power_module_group_id[1];
} channel_power_module_group_bind_item_info_t;

//一个电源管理组内的通道绑定信息
typedef struct {
	uint8_t power_manager_group_id;
	uint8_t size;
	channel_power_module_group_bind_item_info_t **channel_power_module_group_bind_item_info;
} group_channel_power_module_group_bind_info_t;

//所有电源管理组信息
typedef struct {
	uint8_t size;
	group_channel_power_module_group_bind_info_t **group_channel_power_module_group_bind_info;
} groups_channel_power_module_group_bind_info_t;

static channel_power_module_group_bind_item_info_t channel_power_module_group_bind_item_info_0_0 = {
	.channel_id = 0,
	.power_module_group_id = {0},
};

static channel_power_module_group_bind_item_info_t channel_power_module_group_bind_item_info_0_1 = {
	.channel_id = 1,
	.power_module_group_id = {1},
};

static channel_power_module_group_bind_item_info_t channel_power_module_group_bind_item_info_0_2 = {
	.channel_id = 2,
	.power_module_group_id = {2},
};

static channel_power_module_group_bind_item_info_t channel_power_module_group_bind_item_info_0_3 = {
	.channel_id = 3,
	.power_module_group_id = {3},
};

static channel_power_module_group_bind_item_info_t channel_power_module_group_bind_item_info_0_4 = {
	.channel_id = 4,
	.power_module_group_id = {4},
};

static channel_power_module_group_bind_item_info_t *channel_power_module_group_bind_item_info_sz[] = {
	&channel_power_module_group_bind_item_info_0_0,
	&channel_power_module_group_bind_item_info_0_1,
	&channel_power_module_group_bind_item_info_0_2,
	&channel_power_module_group_bind_item_info_0_3,
	&channel_power_module_group_bind_item_info_0_4,
};

static group_channel_power_module_group_bind_info_t group_channel_power_module_group_bind_info_0 = {
	.power_manager_group_id = 0,
	.size = ARRAY_SIZE(channel_power_module_group_bind_item_info_sz),
	.channel_power_module_group_bind_item_info = channel_power_module_group_bind_item_info_sz,
};

static group_channel_power_module_group_bind_info_t *group_channel_power_module_group_bind_info_sz[] = {
	&group_channel_power_module_group_bind_info_0,
};

static groups_channel_power_module_group_bind_info_t groups_channel_power_module_group_bind_info = {
	.size = ARRAY_SIZE(group_channel_power_module_group_bind_info_sz),
	.group_channel_power_module_group_bind_info = group_channel_power_module_group_bind_info_sz,
};

static channel_power_module_group_bind_item_info_t *get_channel_power_module_group_bind_item_info(uint8_t power_manager_group_id, uint8_t channel_id)
{
	int i;
	uint8_t found = 0;
	channel_power_module_group_bind_item_info_t *channel_power_module_group_bind_item_info = NULL;
	group_channel_power_module_group_bind_info_t *group_channel_power_module_group_bind_info = NULL;

	for(i = 0; i < groups_channel_power_module_group_bind_info.size; i++) {
		group_channel_power_module_group_bind_info_t *group_channel_power_module_group_bind_info_item = groups_channel_power_module_group_bind_info.group_channel_power_module_group_bind_info[i];

		if(group_channel_power_module_group_bind_info_item->power_manager_group_id == power_manager_group_id) {
			group_channel_power_module_group_bind_info = group_channel_power_module_group_bind_info_item;
			found = 1;
			break;
		}
	}

	if(found == 0) {
		return channel_power_module_group_bind_item_info;
	}

	found = 0;

	for(i = 0; i < group_channel_power_module_group_bind_info->size; i++) {
		channel_power_module_group_bind_item_info_t *channel_power_module_group_bind_item_info_item = group_channel_power_module_group_bind_info->channel_power_module_group_bind_item_info[i];

		if(channel_power_module_group_bind_item_info_item->channel_id == channel_id) {
			channel_power_module_group_bind_item_info = channel_power_module_group_bind_item_info_item;
			found = 1;
			break;
		}
	}

	return channel_power_module_group_bind_item_info;
}


typedef struct {
	uint8_t channel_id;
	GPIO_TypeDef *gpio_port_fb;
	uint16_t gpio_pin_fb;
	ADC_HandleTypeDef *hadc;
	uint8_t rank;
} channel_relay_fb_node_info_t;

typedef struct {
	uint8_t power_manager_group_id;
	uint8_t size;
	channel_relay_fb_node_info_t **channel_relay_fb_node_info;
} power_manager_group_channel_relay_fb_info_t;

typedef struct {
	uint8_t size;
	power_manager_group_channel_relay_fb_info_t **power_manager_group_channel_relay_fb_info;
} channel_relay_fb_info_t;

static channel_relay_fb_node_info_t channel_relay_fb_node_info_0 = {
	.channel_id = 0,
	.gpio_port_fb = fb6_GPIO_Port,
	.gpio_pin_fb = fb6_Pin,
	.hadc = &hadc1,
	.rank = 0,
};

static channel_relay_fb_node_info_t channel_relay_fb_node_info_1 = {
	.channel_id = 1,
	.gpio_port_fb = fb7_GPIO_Port,
	.gpio_pin_fb = fb7_Pin,
	.hadc = &hadc1,
	.rank = 1,
};

static channel_relay_fb_node_info_t channel_relay_fb_node_info_2 = {
	.channel_id = 2,
	.gpio_port_fb = fb8_GPIO_Port,
	.gpio_pin_fb = fb8_Pin,
	.hadc = &hadc1,
	.rank = 2,
};

static channel_relay_fb_node_info_t channel_relay_fb_node_info_3 = {
	.channel_id = 3,
	.gpio_port_fb = fb9_GPIO_Port,
	.gpio_pin_fb = fb9_Pin,
	.hadc = &hadc1,
	.rank = 3,
};

static channel_relay_fb_node_info_t channel_relay_fb_node_info_4 = {
	.channel_id = 4,
	.gpio_port_fb = fb10_GPIO_Port,
	.gpio_pin_fb = fb10_Pin,
	.hadc = &hadc1,
	.rank = 4,
};

static channel_relay_fb_node_info_t *channel_relay_fb_node_info_sz[] = {
	&channel_relay_fb_node_info_0,
	&channel_relay_fb_node_info_1,
	&channel_relay_fb_node_info_2,
	&channel_relay_fb_node_info_3,
	&channel_relay_fb_node_info_4,
};

static power_manager_group_channel_relay_fb_info_t power_manager_group_channel_relay_fb_info_0 = {
	.power_manager_group_id = 0,
	.size = ARRAY_SIZE(channel_relay_fb_node_info_sz),
	.channel_relay_fb_node_info = channel_relay_fb_node_info_sz,
};

static power_manager_group_channel_relay_fb_info_t *power_manager_group_channel_relay_fb_info_sz[] = {
	&power_manager_group_channel_relay_fb_info_0,
};

static channel_relay_fb_info_t channel_relay_fb_info = {
	.size = ARRAY_SIZE(power_manager_group_channel_relay_fb_info_sz),
	.power_manager_group_channel_relay_fb_info = power_manager_group_channel_relay_fb_info_sz,
};

channel_relay_fb_node_info_t *get_channel_relay_fb_node_info(uint8_t power_manager_group_id, uint8_t channel_id)
{
	channel_relay_fb_node_info_t *channel_relay_fb_node_info = NULL;
	power_manager_group_channel_relay_fb_info_t *power_manager_group_channel_relay_fb_info = NULL;
	int i;

	for(i = 0; i < channel_relay_fb_info.size; i++) {
		power_manager_group_channel_relay_fb_info_t *power_manager_group_channel_relay_fb_info_item = channel_relay_fb_info.power_manager_group_channel_relay_fb_info[i];

		if(power_manager_group_channel_relay_fb_info_item->power_manager_group_id == power_manager_group_id) {
			power_manager_group_channel_relay_fb_info = power_manager_group_channel_relay_fb_info_item;
			break;
		}
	}

	if(power_manager_group_channel_relay_fb_info == NULL) {
		return channel_relay_fb_node_info;
	}

	for(i = 0; i < power_manager_group_channel_relay_fb_info->size; i++) {
		channel_relay_fb_node_info_t *channel_relay_fb_node_info_item = power_manager_group_channel_relay_fb_info->channel_relay_fb_node_info[i];

		if(channel_relay_fb_node_info_item->channel_id == channel_id) {
			channel_relay_fb_node_info = channel_relay_fb_node_info_item;
			break;
		}
	}

	return channel_relay_fb_node_info;
}

static int check_channel_relay_fb_sync(power_manager_group_info_t *power_manager_group_info)
{
	int ret = 0;
	power_manager_info_t *power_manager_info = (power_manager_info_t *)power_manager_group_info->power_manager_info;
	channels_info_t *channels_info = power_manager_info->channels_info;
	int i;
	channel_relay_fb_node_info_t *channel_relay_fb_node_info;

	for(i = 0; i < channels_info->channel_number; i++) {
		power_manager_channel_info_t *power_manager_channel_info = power_manager_info->power_manager_channel_info + i;

		if(power_manager_channel_info->power_manager_group_info != power_manager_group_info) {
			continue;
		}

		if(list_contain(&power_manager_channel_info->list, &power_manager_group_info->channel_active_list) == 0) {
			continue;
		}

		channel_relay_fb_node_info = get_channel_relay_fb_node_info(power_manager_group_info->id, power_manager_channel_info->id);
		OS_ASSERT(channel_relay_fb_node_info != NULL);

		if(HAL_GPIO_ReadPin(channel_relay_fb_node_info->gpio_port_fb, channel_relay_fb_node_info->gpio_pin_fb) == GPIO_PIN_SET) {
			//debug("channel %d fb error!", channel_relay_fb_node_info->channel_id);
			ret = -1;
			break;
		}
	}

	return ret;
}

typedef struct {
	bitmap_t *relay_map;//链式搭接开关状态位
} power_manager_group_policy_ctx_t;

static void clean_up_relay_map(power_manager_group_info_t *power_manager_group_info)
{
	int i;
	power_manager_info_t *power_manager_info = (power_manager_info_t *)power_manager_group_info->power_manager_info;
	power_manager_group_policy_ctx_t *power_manager_group_policy_ctx = (power_manager_group_policy_ctx_t *)power_manager_info->power_manager_group_policy_ctx + power_manager_group_info->id;
	bitmap_t *relay_map = power_manager_group_policy_ctx->relay_map;

	for(i = 0; i < power_manager_group_info->channel_number; i++) {
		set_bitmap_value(relay_map, i, 0);
	}
}

static void action_relay_map(power_manager_group_info_t *power_manager_group_info)
{
	int i;
	power_manager_info_t *power_manager_info = (power_manager_info_t *)power_manager_group_info->power_manager_info;
	power_manager_group_policy_ctx_t *power_manager_group_policy_ctx = (power_manager_group_policy_ctx_t *)power_manager_info->power_manager_group_policy_ctx + power_manager_group_info->id;
	bitmap_t *relay_map = power_manager_group_policy_ctx->relay_map;

	for(i = 0; i < relay_map->size; i++) {
		relay_node_info_t *relay_node_info = get_relay_node_info_by_relay_id(power_manager_group_info->id, i);
		GPIO_PinState state = GPIO_PIN_RESET;

		if(get_bitmap_value(relay_map, i) != 0) {
			state = GPIO_PIN_SET;
		}

		HAL_GPIO_WritePin(relay_node_info->gpio_port, relay_node_info->gpio_pin, state);
	}
}

static int init_average(void *_power_manager_info)
{
	int ret = 0;
	return ret;
}

static int deinit_average(void *_power_manager_info)
{
	int ret = 0;
	return ret;
}

static int channel_start_average(void *_power_manager_channel_info)
{
	int ret = 0;
	power_manager_channel_info_t *power_manager_channel_info = (power_manager_channel_info_t *)_power_manager_channel_info;

	power_manager_channel_info->status.reassign = 0;
	power_manager_channel_info->status.reassign_module_group_number = 0;
	return ret;
}

static int channel_charging_average(void *_power_manager_channel_info)
{
	int ret = 0;
	power_manager_channel_info_t *power_manager_channel_info = (power_manager_channel_info_t *)_power_manager_channel_info;
	uint32_t ticks = osKernelSysTick();

	if(ticks_duration(ticks, power_manager_channel_info->output_current_alive_stamp) > 3000) {
		power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)power_manager_channel_info->power_manager_group_info;

		if(list_size(&power_manager_channel_info->power_module_group_list) > 1) {
			power_module_group_info_t *power_module_group_info = list_first_entry(&power_manager_channel_info->power_module_group_list, power_module_group_info_t, list);
			power_module_item_info_t *power_module_item_info;
			struct list_head *head1 = &power_module_group_info->power_module_item_list;
			list_for_each_entry(power_module_item_info, head1, power_module_item_info_t, list) {
				power_module_item_info->status.state = POWER_MODULE_ITEM_STATE_PREPARE_DEACTIVE;
			}
			list_move_tail(&power_module_group_info->list, &power_manager_group_info->power_module_group_deactive_list);
			debug("remove module group %d from channel %d", power_module_group_info->id, power_manager_channel_info->id);
			power_manager_channel_info->output_current_alive_stamp = ticks;
		}
	}

	return ret;
}

static void channel_info_deactive_power_module_group(power_manager_channel_info_t *power_manager_channel_info)
{
	struct list_head *pos;
	struct list_head *n;
	struct list_head *head;
	power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)power_manager_channel_info->power_manager_group_info;

	head = &power_manager_channel_info->power_module_group_list;

	list_for_each_safe(pos, n, head) {
		power_module_group_info_t *power_module_group_info = list_entry(pos, power_module_group_info_t, list);
		power_module_item_info_t *power_module_item_info;
		struct list_head *head1 = &power_module_group_info->power_module_item_list;
		list_for_each_entry(power_module_item_info, head1, power_module_item_info_t, list) {
			power_module_item_info->status.state = POWER_MODULE_ITEM_STATE_PREPARE_DEACTIVE;
		}
		list_move_tail(&power_module_group_info->list, &power_manager_group_info->power_module_group_deactive_list);
		debug("remove module group %d from channel %d", power_module_group_info->id, power_manager_channel_info->id);
	}
}

static void free_power_module_group_for_stop_channel(power_manager_group_info_t *power_manager_group_info)
{
	power_manager_channel_info_t *power_manager_channel_info;
	struct list_head *head;

	head = &power_manager_group_info->channel_deactive_list;

	list_for_each_entry(power_manager_channel_info, head, power_manager_channel_info_t, list) {
		channel_info_deactive_power_module_group(power_manager_channel_info);
	}
}

static int free_average(void *_power_manager_group_info)
{
	int ret = -1;
	power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)_power_manager_group_info;

	//释放要停机通道的模块
	free_power_module_group_for_stop_channel(power_manager_group_info);
	ret = 0;
	return ret;
}

static void channel_info_assign_one_power_module_group_agerage(power_manager_channel_info_t *power_manager_channel_info)
{
	power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)power_manager_channel_info->power_manager_group_info;
	struct list_head *head;
	power_module_group_info_t *power_module_group_info_item;
	power_module_item_info_t *power_module_item_info;

	if(list_empty(&power_manager_group_info->power_module_group_idle_list)) {
		return;
	}

	power_module_group_info_item = list_first_entry(&power_manager_group_info->power_module_group_idle_list, power_module_group_info_t, list);

	head = &power_module_group_info_item->power_module_item_list;
	list_for_each_entry(power_module_item_info, head, power_module_item_info_t, list) {
		if(power_module_item_info->status.state != POWER_MODULE_ITEM_STATE_IDLE) {
			debug("power module state is not idle:%s!!!", get_power_module_item_state_des(power_module_item_info->status.state));
		}

		power_module_item_info->status.state = POWER_MODULE_ITEM_STATE_PREPARE_ACTIVE;
	}
	power_module_group_info_item->power_manager_channel_info = power_manager_channel_info;
	list_move_tail(&power_module_group_info_item->list, &power_manager_channel_info->power_module_group_list);
	debug("assign module group %d to channel %d", power_module_group_info_item->id, power_manager_channel_info->id);
}

static void active_power_manager_group_info_power_module_group_assign_average(power_manager_group_info_t *power_manager_group_info)
{
	power_manager_channel_info_t *power_manager_channel_info;
	struct list_head *head;

	while(list_size(&power_manager_group_info->power_module_group_idle_list) > 0) {//没有多余的模块需要分配了,退出
		head = &power_manager_group_info->channel_active_list;

		list_for_each_entry(power_manager_channel_info, head, power_manager_channel_info_t, list) {
			channel_info_assign_one_power_module_group_agerage(power_manager_channel_info);
		}
	}
}

static int assign_average(void *_power_manager_group_info)
{
	int ret = 0;
	power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)_power_manager_group_info;
	//充电中的枪数
	uint8_t active_channel_count;

	//获取需要充电的枪数
	active_channel_count = list_size(&power_manager_group_info->channel_active_list);
	debug("active_channel_count:%d", active_channel_count);

	if(active_channel_count == 0) {//如果没有枪需要充电,不分配
		return ret;
	}

	active_power_manager_group_info_power_module_group_assign_average(power_manager_group_info);
	ret = 0;
	return ret;
}

static int _config(void *_power_manager_group_info)
{
	int ret = 0;
	//power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)_power_manager_group_info;
	//power_manager_info_t *power_manager_info = (power_manager_info_t *)power_manager_group_info->power_manager_info;
	//channels_info_t *channels_info = (channels_info_t *)power_manager_info->channels_info;

	return ret;
}

static int _sync(void *_power_manager_group_info)
{
	int ret = 0;
	//power_manager_group_info_t *power_manager_group_info = (power_manager_group_info_t *)_power_manager_group_info;
	//power_manager_info_t *power_manager_info = (power_manager_info_t *)power_manager_group_info->power_manager_info;
	//channels_info_t *channels_info = (channels_info_t *)power_manager_info->channels_info;
	return ret;
}

static power_manager_group_policy_handler_t power_manager_group_policy_handler_average = {
	.policy = POWER_MANAGER_GROUP_POLICY_AVERAGE,
	.init = init_average,
	.deinit = deinit_average,
	.channel_start = channel_start_average,
	.channel_charging = channel_charging_average,
	.free = free_average,
	.assign = assign_average,
	.config = _config,
	.sync = _sync,
};

static power_manager_group_policy_handler_t *power_manager_group_policy_handler_sz[] = {
	&power_manager_group_policy_handler_average,
};

power_manager_group_policy_handler_t *get_power_manager_group_policy_handler(uint8_t policy)
{
	int i;
	power_manager_group_policy_handler_t *power_manager_group_policy_handler = NULL;

	for(i = 0; i < ARRAY_SIZE(power_manager_group_policy_handler_sz); i++) {
		power_manager_group_policy_handler_t *power_manager_group_policy_handler_item = power_manager_group_policy_handler_sz[i];

		if(power_manager_group_policy_handler_item->policy == policy) {
			power_manager_group_policy_handler = power_manager_group_policy_handler_item;
		}
	}

	return power_manager_group_policy_handler;
}

void power_manager_restore_config(channels_info_t *channels_info)
{
	int i;
	int j;

	channels_settings_t *channels_settings = &channels_info->channels_settings;
	power_manager_settings_t *power_manager_settings = &channels_settings->power_manager_settings;

	power_manager_settings->power_manager_group_number = 1;

	debug("power_manager_group_number:%d", power_manager_settings->power_manager_group_number);

	for(i = 0; i < power_manager_settings->power_manager_group_number; i++) {
		power_manager_group_settings_t *power_manager_group_settings = &power_manager_settings->power_manager_group_settings[i];

		power_manager_group_settings->channel_number = 5;
		power_manager_group_settings->relay_board_number_per_channel = 0;

		for(j = 0; j < power_manager_group_settings->relay_board_number_per_channel; j++) {
			power_manager_group_settings->slot_per_relay_board[j] = 6;
		}

		power_manager_group_settings->power_module_group_number = 5;

		channels_info->channel_number += power_manager_group_settings->channel_number;

		for(j = 0; j < power_manager_group_settings->power_module_group_number; j++) {
			power_module_group_settings_t *power_module_group_settings = &power_manager_group_settings->power_module_group_settings[j];
			power_module_group_settings->power_module_number = 3;
		}
	}
}

