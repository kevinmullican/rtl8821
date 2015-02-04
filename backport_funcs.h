#ifndef LINUX_BACKPORT_FUNCS_H
#define LINUX_BACKPORT_FUNCS_H

#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/random.h>

#include <linux/types.h>

#include <linux/pci.h>
#include <linux/dma-mapping.h>

#include <asm/unaligned.h>
#include <asm/bitsperlong.h>

#include <asm/byteorder.h>


/*
 * this function will appear in 
 *
 * include/linux/etherdevice.h
 */

/**
 * ether_addr_equal_unaligned - Compare two not u16 aligned Ethernet addresses
 * @addr1: Pointer to a six-byte array containing the Ethernet address
 * @addr2: Pointer other six-byte array containing the Ethernet address
 *
 * Compare two Ethernet addresses, returns true if equal
 *
 * Please note: Use only when any Ethernet address may not be u16 aligned.
 */
static inline bool ether_addr_equal_unaligned(const u8 *addr1, const u8 *addr2)
{
#if defined(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)
        return ether_addr_equal(addr1, addr2);
#else
        return memcmp(addr1, addr2, ETH_ALEN) == 0;
#endif
}


/*
 * this function will appear in 
 *
 * include/linux/etherdevice.h
 */

/**
 * ether_addr_copy - Copy an Ethernet address
 * @dst: Pointer to a six-byte array Ethernet address destination
 * @src: Pointer to a six-byte array Ethernet address source
 *
 * Please note: dst & src must both be aligned to u16.
 */
static inline void ether_addr_copy(u8 *dst, const u8 *src)
{
#if defined(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)
        *(u32 *)dst = *(const u32 *)src;
        *(u16 *)(dst + 4) = *(const u16 *)(src + 4);
#else
        u16 *a = (u16 *)dst;
        const u16 *b = (const u16 *)src;

        a[0] = b[0];
        a[1] = b[1];
        a[2] = b[2];
#endif
}


/*
 * this function will appear in
 *
 * include/linux/ieee80211.h
 */

/**
 * _ieee80211_is_robust_mgmt_frame - check if frame is a robust management frame
 * @hdr: the frame (buffer must include at least the first octet of payload)
 */
static inline bool _ieee80211_is_robust_mgmt_frame(struct ieee80211_hdr *hdr)
{
        if (ieee80211_is_disassoc(hdr->frame_control) ||
            ieee80211_is_deauth(hdr->frame_control))
                return true;

        if (ieee80211_is_action(hdr->frame_control)) {
                u8 *category;

                /*
                 * Action frames, excluding Public Action frames, are Robust
                 * Management Frames. However, if we are looking at a Protected
                 * frame, skip the check since the data may be encrypted and
                 * the frame has already been found to be a Robust Management
                 * Frame (by the other end).
                 */
                if (ieee80211_has_protected(hdr->frame_control))
                        return true;
                category = ((u8 *) hdr) + 24;
                return *category != WLAN_CATEGORY_PUBLIC &&
                        *category != WLAN_CATEGORY_HT &&
                        *category != WLAN_CATEGORY_SELF_PROTECTED &&
                        *category != WLAN_CATEGORY_VENDOR_SPECIFIC;
        }

        return false;
}


/*
 * this function will appear in
 *
 * include/asm-generic/pci-dma-compat.h
 */

static inline void *
pci_zalloc_consistent(struct pci_dev *hwdev, size_t size,
                      dma_addr_t *dma_handle)
{
        return dma_zalloc_coherent(hwdev == NULL ? NULL : &hwdev->dev,
                                   size, dma_handle, GFP_ATOMIC);
}

#endif //LINUX_BACKPORT_FUNCS_H
