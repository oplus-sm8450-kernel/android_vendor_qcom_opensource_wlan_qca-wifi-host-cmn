/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef _HIF_INTERNAL_H_
#define _HIF_INTERNAL_H_

#include "athdefs.h"
#include "a_types.h"
#include "a_osapi.h"
#include <qdf_types.h>          /* qdf_device_t, qdf_print */
#include <qdf_time.h>           /* qdf_system_ticks, etc. */
#include <qdf_status.h>
#include <qdf_timer.h>
#include <qdf_atomic.h>
#include "hif.h"
#include "hif_debug.h"
#include "hif_sdio_common.h"
#include <linux/scatterlist.h>
#include "hif_main.h"

#define HIF_LINUX_MMC_SCATTER_SUPPORT

#define BUS_REQUEST_MAX_NUM                64

#define SDIO_CLOCK_FREQUENCY_DEFAULT       25000000
#define SDWLAN_ENABLE_DISABLE_TIMEOUT      20
#define FLAGS_CARD_ENAB                    0x02
#define FLAGS_CARD_IRQ_UNMSK               0x04

#define HIF_MBOX_BLOCK_SIZE                HIF_DEFAULT_IO_BLOCK_SIZE
#define HIF_MBOX0_BLOCK_SIZE               1
#define HIF_MBOX1_BLOCK_SIZE               HIF_MBOX_BLOCK_SIZE
#define HIF_MBOX2_BLOCK_SIZE               HIF_MBOX_BLOCK_SIZE
#define HIF_MBOX3_BLOCK_SIZE               HIF_MBOX_BLOCK_SIZE

/*
 * direction - Direction of transfer (HIF_SDIO_READ/HIF_SDIO_WRITE).
 */
#define HIF_SDIO_READ			0x00000001
#define HIF_SDIO_WRITE			0x00000002
#define HIF_SDIO_DIR_MASK		(HIF_SDIO_READ | HIF_SDIO_WRITE)

/*
 * type - An interface may support different kind of rd/wr commands.
 * For example: SDIO supports CMD52/CMD53s. In case of MSIO it
 * translates to using different kinds of TPCs. The command type
 * is thus divided into a basic and an extended command and can
 * be specified using HIF_BASIC_IO/HIF_EXTENDED_IO.
 */
#define HIF_BASIC_IO			0x00000004
#define HIF_EXTENDED_IO			0x00000008
#define HIF_TYPE_MASK			(HIF_BASIC_IO | HIF_EXTENDED_IO)

/*
 * This indicates the whether the command is to be executed in a
 * blocking or non-blocking fashion (HIF_SYNCHRONOUS/
 * HIF_ASYNCHRONOUS). The read/write data paths in HTC have been
 * implemented using the asynchronous mode allowing the the bus
 * driver to indicate the completion of operation through the
 * registered callback routine. The requirement primarily comes
 * from the contexts these operations get called from (a driver's
 * transmit context or the ISR context in case of receive).
 * Support for both of these modes is essential.
 */
#define HIF_SYNCHRONOUS		0x00000010
#define HIF_ASYNCHRONOUS	0x00000020
#define HIF_EMODE_MASK		(HIF_SYNCHRONOUS | HIF_ASYNCHRONOUS)

/*
 * An interface may support different kinds of commands based on
 * the tradeoff between the amount of data it can carry and the
 * setup time. Byte and Block modes are supported (HIF_BYTE_BASIS/
 * HIF_BLOCK_BASIS). In case of latter, the data is rounded off
 * to the nearest block size by padding. The size of the block is
 * configurable at compile time using the HIF_BLOCK_SIZE and is
 * negotiated with the target during initialization after the
 * AR6000 interrupts are enabled.
 */
#define HIF_BYTE_BASIS		0x00000040
#define HIF_BLOCK_BASIS		0x00000080
#define HIF_DMODE_MASK		(HIF_BYTE_BASIS | HIF_BLOCK_BASIS)

/*
 * This indicates if the address has to be incremented on AR6000
 * after every read/write operation (HIF?FIXED_ADDRESS/
 * HIF_INCREMENTAL_ADDRESS).
 */
#define HIF_FIXED_ADDRESS			0x00000100
#define HIF_INCREMENTAL_ADDRESS		0x00000200
#define HIF_AMODE_MASK				(HIF_FIXED_ADDRESS | \
							HIF_INCREMENTAL_ADDRESS)

/*
 * data written into the dummy space will not put into the final mbox FIFO
 */
#define HIF_DUMMY_SPACE_MASK			0xFFFF0000

/*
 * data written into the dummy space will not put into the final mbox FIFO
 */
#define HIF_DUMMY_SPACE_MASK			0xFFFF0000


#define HIF_WR_ASYNC_BYTE_FIX   \
		(HIF_SDIO_WRITE | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BYTE_BASIS | HIF_FIXED_ADDRESS)
#define HIF_WR_ASYNC_BYTE_INC   \
	(HIF_SDIO_WRITE | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BYTE_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_WR_ASYNC_BLOCK_INC  \
	(HIF_SDIO_WRITE | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BLOCK_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_WR_SYNC_BYTE_FIX    \
	(HIF_SDIO_WRITE | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BYTE_BASIS | HIF_FIXED_ADDRESS)
#define HIF_WR_SYNC_BYTE_INC    \
	(HIF_SDIO_WRITE | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BYTE_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_WR_SYNC_BLOCK_INC  \
	(HIF_SDIO_WRITE | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BLOCK_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_WR_ASYNC_BLOCK_FIX \
	(HIF_SDIO_WRITE | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BLOCK_BASIS | HIF_FIXED_ADDRESS)
#define HIF_WR_SYNC_BLOCK_FIX  \
	(HIF_SDIO_WRITE | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BLOCK_BASIS | HIF_FIXED_ADDRESS)
#define HIF_RD_SYNC_BYTE_INC    \
	(HIF_SDIO_READ | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BYTE_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_RD_SYNC_BYTE_FIX    \
	(HIF_SDIO_READ | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BYTE_BASIS | HIF_FIXED_ADDRESS)
#define HIF_RD_ASYNC_BYTE_FIX   \
	(HIF_SDIO_READ | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BYTE_BASIS | HIF_FIXED_ADDRESS)
#define HIF_RD_ASYNC_BLOCK_FIX  \
	(HIF_SDIO_READ | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BLOCK_BASIS | HIF_FIXED_ADDRESS)
#define HIF_RD_ASYNC_BYTE_INC   \
	(HIF_SDIO_READ | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BYTE_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_RD_ASYNC_BLOCK_INC  \
	(HIF_SDIO_READ | HIF_ASYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BLOCK_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_RD_SYNC_BLOCK_INC  \
	(HIF_SDIO_READ | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BLOCK_BASIS | HIF_INCREMENTAL_ADDRESS)
#define HIF_RD_SYNC_BLOCK_FIX  \
	(HIF_SDIO_READ | HIF_SYNCHRONOUS | HIF_EXTENDED_IO | \
				HIF_BLOCK_BASIS | HIF_FIXED_ADDRESS)

enum hif_sdio_device_state {
		HIF_DEVICE_STATE_ON,
		HIF_DEVICE_STATE_DEEPSLEEP,
		HIF_DEVICE_STATE_CUTPOWER,
		HIF_DEVICE_STATE_WOW
};

struct bus_request {
	struct bus_request *next;       /* link list of available requests */
	struct bus_request *inusenext;  /* link list of in use requests */
	struct semaphore sem_req;
	uint32_t address;       /* request data */
	char *buffer;
	uint32_t length;
	uint32_t request;
	void *context;
	QDF_STATUS status;
	struct HIF_SCATTER_REQ_PRIV *scatter_req;
};

struct hif_sdio_dev {
	struct sdio_func *func;
	qdf_spinlock_t asynclock;
	struct task_struct *async_task; /* task to handle async commands */
	struct semaphore sem_async;     /* wake up for async task */
	int async_shutdown;     /* stop the async task */
	struct completion async_completion;     /* thread completion */
	struct bus_request *asyncreq;  /* request for async tasklet */
	struct bus_request *taskreq;   /*  async tasklet data */
	qdf_spinlock_t lock;
	struct bus_request *bus_request_free_queue;     /* free list */
	struct bus_request bus_request[BUS_REQUEST_MAX_NUM]; /* bus requests */
	void *claimed_ctx;
	struct htc_callbacks htc_callbacks;
	uint8_t *dma_buffer;
	DL_LIST scatter_req_head; /* scatter request list head */
	bool scatter_enabled; /* scatter enabled flag */
	bool is_suspend;
	bool is_disabled;
	atomic_t irq_handling;
	HIF_DEVICE_POWER_CHANGE_TYPE power_config;
	enum hif_sdio_device_state device_state;
	const struct sdio_device_id *id;
	struct mmc_host *host;
	void *htc_context;
	bool swap_mailbox;
};

struct HIF_DEVICE_OS_DEVICE_INFO {
	void *os_dev;
};

struct hif_mailbox_properties {
	u_int32_t    extended_address;  /* extended address for larger writes */
	u_int32_t    extended_size;
};

struct hif_device_irq_yield_params {
	int recv_packet_yield_count;
	/* max number of packets to force DSR to return */
};

struct hif_device_mbox_info {
	u_int32_t mbox_addresses[4];
	/* first element for legacy HIFs and return the address and ARRAY of
	 * 32bit words
	 */
	struct hif_mailbox_properties mbox_prop[4];
	u_int32_t gmbox_address;
	u_int32_t gmbox_size;
	u_int32_t flags;
	/* flags to describe mbox behavior or usage */
};

enum hif_device_irq_mode {
	HIF_DEVICE_IRQ_SYNC_ONLY,
	/* DSR to process all interrupts before returning */
	HIF_DEVICE_IRQ_ASYNC_SYNC,  /* DSR to process interrupts */
};

struct osdrv_callbacks {
	void *context;
	/* context to pass for all callbacks
	 * except device_removed_handler
	 * the device_removed_handler is only
	 * called if the device is claimed
	 */
	int (*device_inserted_handler)(void *context, void *hif_handle);
	int (*device_removed_handler)(void *claimed_ctx,
				    void *hif_handle);
	int (*device_suspend_handler)(void *context);
	int (*device_resume_handler)(void *context);
	int (*device_wakeup_handler)(void *context);
	int (*device_power_change_handler)(void *context,
					HIF_DEVICE_POWER_CHANGE_TYPE
					config);
};

/* other interrupts are pending, host
 * needs to read the to monitor
 */
#define HIF_OTHER_EVENTS     (1 << 0)
/* pending recv packet */
#define HIF_RECV_MSG_AVAIL   (1 << 1)

struct _HIF_PENDING_EVENTS_INFO {
	uint32_t events;
	uint32_t look_ahead;
	uint32_t available_recv_bytes;
};

/* hif-sdio pending events handler type, some HIF modules
 * use special mechanisms to detect packet available and other interrupts
 */
typedef int (*HIF_PENDING_EVENTS_FUNC)(struct hif_sdio_dev *device,
					struct _HIF_PENDING_EVENTS_INFO *
					events, void *async_context);

#define HIF_MASK_RECV    true
#define HIF_UNMASK_RECV  false
/* hif-sdio Handler type to mask receive events */
typedef int (*HIF_MASK_UNMASK_RECV_EVENT)(struct hif_sdio_dev *device,
					  bool mask,
					  void *async_context);

QDF_STATUS hif_configure_device(struct hif_sdio_dev *device,
			enum hif_device_config_opcode opcode,
			void *config, uint32_t config_len);

QDF_STATUS hif_init(struct osdrv_callbacks *callbacks);

QDF_STATUS hif_attach_htc(struct hif_sdio_dev *device,
			  HTC_CALLBACKS *callbacks);

QDF_STATUS hif_read_write(struct hif_sdio_dev *device,
			uint32_t address,
			char *buffer,
			uint32_t length, uint32_t request, void *context);

void hif_ack_interrupt(struct hif_sdio_dev *device);

void hif_mask_interrupt(struct hif_sdio_dev *device);

void hif_un_mask_interrupt(struct hif_sdio_dev *device);

QDF_STATUS hif_wait_for_pending_recv(struct hif_sdio_dev *device);

struct _HIF_SCATTER_ITEM {
	u_int8_t     *buffer; /* CPU accessible address of buffer */
	int          length; /* length of transfer to/from this buffer */
	void         *caller_contexts[2]; /* caller context */
};

struct _HIF_SCATTER_REQ;

typedef void (*HIF_SCATTER_COMP_CB)(struct _HIF_SCATTER_REQ *);

enum HIF_SCATTER_METHOD {
	HIF_SCATTER_NONE = 0,
	HIF_SCATTER_DMA_REAL, /* Real SG support no restrictions */
	HIF_SCATTER_DMA_BOUNCE, /* Uses SG DMA */
};

struct _HIF_SCATTER_REQ {
	DL_LIST             list_link; /* link management */
	u_int32_t            address; /* address for the read/write operation */
	u_int32_t            request; /* request flags */
	u_int32_t            total_length; /* total length of entire transfer */
	u_int32_t            caller_flags; /* caller specific flags */
	HIF_SCATTER_COMP_CB  completion_routine; /* completion callback */
	int                  completion_status; /* status of completion */
	void                 *context; /* caller context for this request */
	int                  valid_scatter_entries; /* no of valid entries */
	/* scatter method handled by HIF */
	enum HIF_SCATTER_METHOD   scatter_method;
	void                 *hif_private[4]; /* HIF private area */
	u_int8_t             *scatter_bounce_buffer; /* bounce buffers */
	struct _HIF_SCATTER_ITEM    scatter_list[1]; /* start of scatter list */
};

typedef struct _HIF_SCATTER_REQ * (*HIF_ALLOCATE_SCATTER_REQUEST)(
						struct hif_sdio_dev *device);
typedef void (*HIF_FREE_SCATTER_REQUEST)(struct hif_sdio_dev *device,
				struct _HIF_SCATTER_REQ *request);
typedef QDF_STATUS (*HIF_READWRITE_SCATTER)(struct hif_sdio_dev *device,
					struct _HIF_SCATTER_REQ *request);

struct HIF_DEVICE_SCATTER_SUPPORT_INFO {
	/* information returned from HIF layer */
	HIF_ALLOCATE_SCATTER_REQUEST    allocate_req_func;
	HIF_FREE_SCATTER_REQUEST        free_req_func;
	HIF_READWRITE_SCATTER           read_write_scatter_func;
	int                             max_scatter_entries;
	int                             max_tx_size_per_scatter_req;
};

void hif_get_target_revision(struct hif_softc *ol_sc);
struct HIF_SCATTER_REQ_PRIV;

#define HIF_DMA_BUFFER_SIZE (4 * 1024)
#define CMD53_FIXED_ADDRESS 1
#define CMD53_INCR_ADDRESS  2

struct bus_request *hif_allocate_bus_request(struct hif_sdio_dev *device);
void hif_free_bus_request(struct hif_sdio_dev *device,
			  struct bus_request *busrequest);
void add_to_async_list(struct hif_sdio_dev *device,
		       struct bus_request *busrequest);
void hif_dump_cccr(struct hif_sdio_dev *hif_device);

#ifdef HIF_LINUX_MMC_SCATTER_SUPPORT

#define MAX_SCATTER_REQUESTS             4
#define MAX_SCATTER_ENTRIES_PER_REQ      16
#define MAX_SCATTER_REQ_TRANSFER_SIZE    (32*1024)

struct HIF_SCATTER_REQ_PRIV {
	struct _HIF_SCATTER_REQ *hif_scatter_req;
	struct hif_sdio_dev *device;     /* this device */
	struct bus_request *busrequest;
	/* scatter list for linux */
	struct scatterlist sgentries[MAX_SCATTER_ENTRIES_PER_REQ];
};

#define ATH_DEBUG_SCATTER  ATH_DEBUG_MAKE_MODULE_MASK(0)

QDF_STATUS setup_hif_scatter_support(struct hif_sdio_dev *device,
		   struct HIF_DEVICE_SCATTER_SUPPORT_INFO *info);
void cleanup_hif_scatter_resources(struct hif_sdio_dev *device);
QDF_STATUS do_hif_read_write_scatter(struct hif_sdio_dev *device,
				   struct bus_request *busrequest);

#else                           /* HIF_LINUX_MMC_SCATTER_SUPPORT */

static inline QDF_STATUS setup_hif_scatter_support(struct hif_sdio_dev *device,
				struct HIF_DEVICE_SCATTER_SUPPORT_INFO *info)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS do_hif_read_write_scatter(struct hif_sdio_dev *device,
					 struct bus_request *busrequest)
{
	return QDF_STATUS_E_NOSUPPORT;
}

#define cleanup_hif_scatter_resources(d) { }

#endif /* HIF_LINUX_MMC_SCATTER_SUPPORT */

#endif /* _HIF_INTERNAL_H_ */
