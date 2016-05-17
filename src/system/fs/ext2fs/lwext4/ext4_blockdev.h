/*
 * Copyright (c) 2013 Grzegorz Kostka (kostka.grzegorz@gmail.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef EXT4_BLOCKDEV_H_
#define EXT4_BLOCKDEV_H_

/** @addtogroup lwext4
 * @{
 */
/**
 * @file  ext4_blockdev.h
 * @brief Block device module.
 */

#include <ext4_config.h>
#include <ext4_bcache.h>
#include <ext4_debug.h>


#include <stdbool.h>
#include <stdint.h>

/**@brief   Initialization status flag*/
#define EXT4_BDEV_INITIALIZED   (1 << 0)

/** @brief block device interface. */
struct ext4_blockdev;
struct ext4_os_if {
    /**@brief   Block read function.
     * @param   bdev block device
     * @param   buf output buffer
     * @param   blk_id block id
     * @param   blk_cnt block count*/
    int (*bread)(struct ext4_blockdev *bdev, void *buf, uint64_t blk_id, uint32_t blk_cnt);

    /**@brief   Block write function.
     * @param   buf input buffer
     * @param   blk_id block id
     * @param   blk_cnt block count*/
    int (*bwrite)(struct ext4_blockdev *bdev, const void *buf, uint64_t blk_id, uint32_t blk_cnt);

    /**@brief   Lock access to mount point (should be recursive) */
    void (*lock)(void *ctx);

    /**@brief   Unlock access to mount point (should be recursive)*/
    void (*unlock)(void *ctx);
};

/**@brief   Definiton of the simple block device.*/
struct ext4_blockdev {
    /**@brief   User context */
    void        *usr_ctx;

    /**@brief   OS Interface */
    const struct ext4_os_if *osif;

    /**@brief   Block cache.*/
    struct  ext4_bcache *bc;

    /**@brief   Block size (bytes): physical*/
    uint32_t    ph_bsize;

    /**@brief   Block count: physical.*/
    uint64_t    ph_bcnt;

    /**@brief   Block size buffer: physical.*/
    uint8_t     *ph_bbuf;

    /**@brief   Block size (bytes) logical*/
    uint32_t    lg_bsize;

    /**@brief   Block count: phisical.*/
    uint64_t    lg_bcnt;

    /**@brief   Cache write back mode refference counter.*/
    uint32_t    cache_write_back;

    /**@brief   Physical read counter*/
    uint32_t    bread_ctr;

    /**@brief   Physical write counter*/
    uint32_t    bwrite_ctr;
};

/**@brief   Block device initialization.
 * @param   bdev block device descriptor
 * @param   osif OS interface
 * @param   usr_ctx user's context
 * @param   ph_bsize physical block size
 * @param   ph_bcnt physical number of blocks
 * @return  standard error code*/
int ext4_block_init(struct ext4_blockdev    *bdev,
                    const struct ext4_os_if *osif,
                    void                    *usr_ctx,
                    uint32_t                 ph_bsize,
                    uint64_t                 ph_bcnt);

/**@brief   Binds a bcache to block device.
 * @param   bdev block device descriptor
 * @param   bc block cache descriptor
 * @return  standard error code*/
int ext4_block_bind_bcache(struct ext4_blockdev *bdev, struct ext4_bcache *bc);

/**@brief   Close block device
 * @param   bdev block device descriptor
 * @return  standard error code*/
int ext4_block_fini(struct ext4_blockdev *bdev);

/**@brief   Set logical block size in block device.
 * @param   bdev block device descriptor
 * @param   lb_size ligical block size (in bytes)
 * @return  standard error code*/
void ext4_block_set_lb_size(struct ext4_blockdev *bdev,
 uint64_t lb_bsize);

/**@brief   Block get function (through cache).
 * @param   bdev block device descriptor
 * @param   b block descriptor
 * @param   lba logical block address
 * @return  standard error code*/
int ext4_block_get(struct ext4_blockdev *bdev, struct ext4_block *b, uint64_t lba);

/**@brief   Block set procedure (through cache).
 * @param   bdev block device descriptor
 * @param   b block descriptor
 * @return  standard error code*/
int ext4_block_set(struct ext4_blockdev *bdev, struct ext4_block *b);


/**@brief   Block read procedure (without cache)
 * @param   bdev block device descriptor
 * @param   buf output buffer
 * @param   lba logical block adderss
 * @return  standard error code*/
int ext4_blocks_get_direct(struct   ext4_blockdev *bdev, void *buf, uint64_t lba, uint32_t cnt);


/**@brief   Block write procedure (without cache)
 * @param   bdev block device descriptor
 * @param   buf output buffer
 * @param   lba logical block address
 * @return  standard error code*/
int ext4_blocks_set_direct(struct ext4_blockdev *bdev, const void *buf, uint64_t lba, uint32_t cnt);

/**@brief   Write to block device (by direct adress).
 * @param   bdev block device descriptor
 * @param   off byte offset in block device
 * @param   buf input buffer
 * @param   len length of the write nuffer
 * @return  EOK when sucess*/
int ext4_block_writebytes(struct ext4_blockdev *bdev, uint64_t off, const void *buf, uint32_t len);



/**@brief   Read freom block device (by direct adress).
 * @param   bdev block device descriptor
 * @param   off byte offset in block device
 * @param   buf input buffer
 * @param   len length of the write nuffer
 * @return  EOK when sucess*/
int ext4_block_readbytes(struct ext4_blockdev *bdev, uint64_t off, void *buf, uint32_t len);

/**@brief   Enable/disable write back cache mode
 * @param   bdev block device descriptor
 * @param   on_off
 *              !0 - ENABLE
 *               0 - DISABLE (all delayed cache buffers will be flushed)
 * @return  standard error code*/
int ext4_block_cache_write_back(struct ext4_blockdev *bdev, uint8_t on_off);

#endif /* EXT4_BLOCKDEV_H_ */

/**
 * @}
 */