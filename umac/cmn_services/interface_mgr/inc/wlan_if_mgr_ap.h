/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: contains interface manager public file containing AP event handlers
 */

#ifndef _WLAN_IF_MGR_AP_H_
#define _WLAN_IF_MGR_AP_H_

/**
 * wlan_process_start_bss_start() - Start BSS event handler
 *
 * Interface manager connect start bss event handler
 *
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_process_ap_start_bss(struct wlan_objmgr_vdev *vdev,
				     struct if_mgr_event_data *event_data);

#endif