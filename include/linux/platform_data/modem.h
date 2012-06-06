/*
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2010 Samsung Electronics.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __MODEM_IF_H__
#define __MODEM_IF_H__

enum modem_t {
	IMC_XMM6260,
	IMC_XMM6262,
	VIA_CBP71,
	VIA_CBP72,
	SEC_CMC221,
	QC_MDM6600,
	DUMMY,
};

enum dev_format {
	IPC_FMT,
	IPC_RAW,
	IPC_RFS,
	IPC_CMD,
	IPC_BOOT,
	IPC_MULTI_RAW,
	IPC_RAMDUMP,
	MAX_DEV_FORMAT,
};

enum modem_io {
	IODEV_MISC,
	IODEV_NET,
	IODEV_DUMMY,
};

enum modem_link {
	LINKDEV_UNDEFINED,
	LINKDEV_MIPI,
	LINKDEV_DPRAM,
	LINKDEV_SPI,
	LINKDEV_USB,
	LINKDEV_HSIC,
	LINKDEV_C2C,
	LINKDEV_MAX,
};
#define LINKTYPE(modem_link) (1u << (modem_link))

enum modem_network {
	UMTS_NETWORK,
	CDMA_NETWORK,
	LTE_NETWORK,
};

enum sipc_ver {
	NO_SIPC_VER = 0,
	SIPC_VER_40,
	SIPC_VER_41,
	SIPC_VER_50,
	MAX_SIPC_VER,
};

enum sipc_dev_type {
	FMT_DEV = 0,
	RAW_DEV,
	RFS_DEV,
	MAX_IPC_DEV,
};

/**
 * struct modem_io_t - declaration for io_device
 * @name:	device name
 * @id:		id. contain channel information if this is IPC_RAW
 *		when IPC_RAW: .id = 0x30 | channel
 * @format:	device format
 * @io_type:	type of this io_device
 * @links:	list of link_devices to use this io_device
 *		for example, if you want to use DPRAM and USB in an io_device.
 *		.links = LINKTYPE(LINKDEV_DPRAM) | LINKTYPE(LINKDEV_USB)
 * @tx_link:	when you use 2+ link_devices, set the link for TX.
 *		If define multiple link_devices in @links,
 *		you can receive data from them. But, cannot send data to all.
 *		TX is only one link_device.
 *
 * This structure is used in board-*-modem.c
 */
struct modem_io_t {
	char *name;
	int   id;
	enum dev_format format;
	enum modem_io   io_type;
	enum modem_link links;
	enum modem_link tx_link;
};

struct modemlink_pm_data {
	char *name;
	/* link power contol 2 types : pin & regulator control */
	int (*link_ldo_enable)(bool);
	unsigned gpio_link_enable;
	unsigned gpio_link_active;
	unsigned gpio_link_hostwake;
	unsigned gpio_link_slavewake;
	int (*link_reconnect)(void);
	int (*port_enable)(int, int);
	int *p_hub_status;
<<<<<<< HEAD

	atomic_t freqlock;
	int (*cpufreq_lock)(void);
	int (*cpufreq_unlock)(void);
=======
>>>>>>> 1369860... Revert "modem_if: n7000 modem driver"
};

struct modemlink_pm_link_activectl {
	int gpio_initialized;
	int gpio_request_host_active;
};

enum dpram_type {
	EXT_DPRAM,
	CP_IDPRAM,
	AP_IDPRAM,
	C2C_DPRAM,
	MAX_DPRAM_TYPE
};

enum dpram_speed {
	DPRAM_SPEED_LOW,
	DPRAM_SPEED_HIGH,
	MAX_DPRAM_SPEED
};

struct modemlink_dpram_control {
	void (*reset)(void);
	void (*setup_speed)(enum dpram_speed);
	int  (*wakeup)(void);
	void (*sleep)(void);

	void (*clear_intr)(void);
	u16  (*recv_intr)(void);
	void (*send_intr)(u16);
	u16  (*recv_msg)(void);
	void (*send_msg)(u16);

	u16  (*get_magic)(void);
	void  (*set_magic)(u16);

	u16  (*get_access)(void);
	void  (*set_access)(u16);

	u32  (*get_tx_head)(int);
	u32  (*get_tx_tail)(int);
	void (*set_tx_head)(int, u32);
	void (*set_tx_tail)(int, u32);
	u8 __iomem * (*get_tx_buff)(int);
	u32  (*get_tx_buff_size)(int);
	u16  (*get_mask_req_ack)(int);
	u16  (*get_mask_res_ack)(int);
	u16  (*get_mask_send)(int);

	u32  (*get_rx_head)(int);
	u32  (*get_rx_tail)(int);
	void (*set_rx_head)(int, u32);
	void (*set_rx_tail)(int, u32);
	u8 __iomem * (*get_rx_buff)(int);
	u32  (*get_rx_buff_size)(int);

	void (*log_disp)(struct modemlink_dpram_control *dpctl);
	int (*cpupload_step1)(struct modemlink_dpram_control *dpctl);
	int (*cpupload_step2)(void *arg, struct modemlink_dpram_control *dpctl);
	int (*cpimage_load_prepare)(struct modemlink_dpram_control *dpctl);
	int (*cpimage_load)(void *arg, struct modemlink_dpram_control *dpctl);
	int (*nvdata_load)(void *arg, struct modemlink_dpram_control *dpctl);
	int (*phone_boot_start)(struct modemlink_dpram_control *dpctl);
	int (*phone_boot_start_post_process)(void);
	void (*phone_boot_start_handler)(struct modemlink_dpram_control *dpctl);
	void (*dload_cmd_hdlr)(
		struct modemlink_dpram_control *dpctl, u16 cmd);
	void (*bt_map_init)(struct modemlink_dpram_control *dpctl);
	void (*load_init)(struct modemlink_dpram_control *dpctl);

	u8 __iomem      *dp_base;
	u32              dp_size;
	enum dpram_type  dp_type;	/* DPRAM type */
	int		 aligned;	/* If aligned access is required, ... */

	int              dpram_irq;
	unsigned long    dpram_irq_flags;
	char            *dpram_irq_name;
	char            *dpram_wlock_name;

	int              max_ipc_dev;
};

#define DPRAM_MAGIC_CODE	0xAA


/* platform data */
struct modem_data {
	char *name;

	unsigned gpio_cp_on;
	unsigned gpio_cp_off;
	unsigned gpio_reset_req_n;
	unsigned gpio_cp_reset;
	unsigned gpio_pda_active;
	unsigned gpio_phone_active;
	unsigned gpio_cp_dump_int;
	unsigned gpio_flm_uart_sel;
	unsigned gpio_cp_warm_reset;
<<<<<<< HEAD
	unsigned gpio_sim_detect;
=======
>>>>>>> 1369860... Revert "modem_if: n7000 modem driver"

#ifdef CONFIG_LTE_MODEM_CMC221
	unsigned gpio_dpram_status;
	unsigned gpio_dpram_wakeup;
	unsigned gpio_slave_wakeup;
	unsigned gpio_host_active;
	unsigned gpio_host_wakeup;
	int      irq_host_wakeup;
#endif
#ifdef CONFIG_MACH_U1_KOR_LGT
	unsigned gpio_cp_reset_msm;
	unsigned gpio_boot_sw_sel;
	void (*vbus_on)(void);
	void (*vbus_off)(void);
	struct regulator *cp_vbus;
#endif

	/* Modem component */
	enum modem_network  modem_net;
	enum modem_t        modem_type;
	enum modem_link     link_types;
	char               *link_name;
#ifdef CONFIG_LINK_DEVICE_DPRAM
	/* Link to DPRAM control functions dependent on each platform */
	struct modemlink_dpram_control *dpram_ctl;
#endif

	/* SIPC version */
	enum sipc_ver ipc_version;

	/* Information of IO devices */
	unsigned            num_iodevs;
	struct modem_io_t  *iodevs;

	/* Modem link PM support */
	struct modemlink_pm_data *link_pm_data;

	void (*gpio_revers_bias_clear)(void);
	void (*gpio_revers_bias_restore)(void);

	/* Handover with 2+ link devices */
	bool use_handover;
};

/* DEBUG */
#define LOG_TAG "mif: "
#define lnk_log(level, lnk, s, args...) \
	printk(level LOG_TAG "%s: %s: " s, lnk->ld.name, __func__, ##args)
#define iod_log(level, iod, s, args...) \
	printk(level LOG_TAG "%s-%s: %s: " s, iod->link->name, iod->name, \
	__func__, ##args)
#define mdm_log(level, mctl, s, args...) \
	printk(level LOG_TAG "%s-%s: %s: " s, mctl->name, mctl->iod->name, \
	__func__, ##args)
#define mif_trace(s, args...) \
	printk(KERN_DEBUG LOG_TAG ": %s: %d: called(%pF): " s, __func__, \
	__LINE__, __builtin_return_address(0), ##args)


#ifdef DEBUG
/* for link device debug log */
#define lnk_dbg(lnk, s, args...)	lnk_log(KERN_DEBUG, lnk, s, ##args)
#define lnk_info(lnk, s, args...)	lnk_log(KERN_INFO, lnk, s, ##args)
#define lnk_err(lnk, s, args...)	lnk_log(KERN_ERR, lnk, s, ##args)
/* for io device debug log */
#define iod_dbg(iod, s, args...)	iod_log(KERN_DEBUG, iod, s, ##args)
#define iod_info(iod, s, args...)	iod_log(KERN_INFO, iod, s, ##args)
#define iod_err(iod, s, args...)	iod_log(KERN_ERR, iod, s, ##args)
/* for modemctl debug log */
#define mdm_dbg(dev, s, args...)	mdm_log(KERN_DEBUG, dev, s, ##args)
#define mdm_info(dev, s, args...)	mdm_log(KERN_INFO, dev, s, ##args)
#define mdm_err(dev, s, args...)	mdm_log(KERN_ERR, dev, s, ##args)
#else
/* for link device ship log */
#define lnk_dbg(lnk, s, args...)	\
			({ if (0) lnk_log(KERN_DEBUG, lnk, s, ##args); 0; })
#define lnk_info(lnk, s, args...)	lnk_log(KERN_DEBUG, lnk, s, ##args)
#define lnk_err(lnk, s, args...)	lnk_log(KERN_ERR, lnk, s, ##args)
/* for io device ship log */
#define iod_dbg(iod, s, args...)	\
			({ if (0) iod_log(KERN_DEBUG, iod, s, ##args); 0; })
#define iod_info(iod, s, args...)	iod_log(KERN_DEBUG, iod, s, ##args)
#define iod_err(iod, s, args...)	iod_log(KERN_ERR, iod, s, ##args)
/* for modemctl debug log */
#define mdm_dbg(dev, s, args...)	\
	({ if (0) mdm_log(KERN_DEBUG, dev, s, ##args); 0; })
#define mdm_info(dev, s, args...)	mdm_log(KERN_DEBUG, dev, s, ##args)
#define mdm_err(dev, s, args...)	mdm_log(KERN_ERR, dev, s, ##args)
#endif

#endif
