#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>

#define MOD_ADLER       65521
#define CHECKSUM_SIZE   8

/*
 * Function prototypes for the adler driver.
 */
static void restore_initial_state();

static int adler_open(devminor_t minor, int access, endpoint_t user_endpt);

static int adler_close(devminor_t minor);

static ssize_t adler_read(devminor_t minor, u64_t position, endpoint_t endpt,
                          cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);

static ssize_t adler_write(devminor_t minor, u64_t position, endpoint_t endpt,
                           cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);

/* SEF functions and variables. */
static void sef_local_startup(void);

static int sef_cb_init(int type, sef_init_info_t *info);

static int sef_cb_lu_state_save(int);

static int lu_state_restore(void);

/* Entry points to the adler driver. */
static struct chardriver adler_tab =
        {
                .cdr_open    = adler_open,
                .cdr_close    = adler_close,
                .cdr_read    = adler_read,
                .cdr_write    = adler_write,
        };

static uint32_t a, b;

static void restore_initial_state() {
    a = 1;
    b = 0;
}

static int adler_open(devminor_t UNUSED(minor), int UNUSED(access),
                      endpoint_t UNUSED(user_endpt)) {
    return OK;
}

static int adler_close(devminor_t UNUSED(minor)) {
    return OK;
}

static ssize_t adler_read(devminor_t UNUSED(minor), u64_t position, endpoint_t endpt,
                          cp_grant_id_t grant, size_t size, int UNUSED(flags), cdev_id_t UNUSED(id)) {
    u64_t dev_size = CHECKSUM_SIZE;
    char *ptr;
    int ret;
    char buf[CHECKSUM_SIZE + 1] = {0};

    if (size < dev_size) {
        return EINVAL;
    }
    if (position >= dev_size) {
        return 0;                               /* EOF */
    }
    if (position + size > dev_size) {
        size = (size_t) (dev_size - position);  /* limit size */
    }

    sprintf(buf, "%08x", (b << 16U) | a);
    restore_initial_state();

    /* Copy the requested part to the caller. */
    ptr = buf + (size_t) position;
    if ((ret = sys_safecopyto(endpt, grant, 0, (vir_bytes) ptr, size)) != OK) {
        return ret;
    }

    /* Return the number of bytes read. */
    return size;
}

static ssize_t adler_write(devminor_t UNUSED(minor), u64_t UNUSED(position), endpoint_t endpt,
                           cp_grant_id_t grant, size_t size, int UNUSED(flags), cdev_id_t UNUSED(id)) {
    int ret;
    size_t buf_size = 256, read_len;
    unsigned char buf[buf_size];

    for (size_t offset = 0; offset < size; offset += read_len) {
        read_len = MIN(size - offset, buf_size);
        if ((ret = sys_safecopyfrom(endpt, grant, offset, (vir_bytes) buf, read_len)) != OK) {
            return ret;
        }
        for (size_t i = 0; i < read_len; ++i) {
            a = (a + buf[i]) % MOD_ADLER;
            b = (b + a) % MOD_ADLER;
        }
    }

    return size;
}

static int sef_cb_lu_state_save(int UNUSED(state)) {
/* Save the state. */
    ds_publish_u32("adler_A", a, DSF_OVERWRITE);
    ds_publish_u32("adler_B", b, DSF_OVERWRITE);

    return OK;
}

static int lu_state_restore() {
/* Restore the state. */
    uint32_t a_val, b_val;

    ds_retrieve_u32("adler_A", &a_val);
    ds_delete_u32("adler_A");
    ds_retrieve_u32("adler_B", &b_val);
    ds_delete_u32("adler_B");
    a = a_val;
    b = b_val;

    return OK;
}

static void sef_local_startup() {
    /*
     * Register init callbacks. Use the same function for all event types
     */
    sef_setcb_init_fresh(sef_cb_init);
    sef_setcb_init_lu(sef_cb_init);
    sef_setcb_init_restart(sef_cb_init);

    /*
     * Register live update callbacks.
     */
    /* - Agree to update immediately when LU is requested in a valid state. */
    sef_setcb_lu_prepare(sef_cb_lu_prepare_always_ready);
    /* - Support live update starting from any standard state. */
    sef_setcb_lu_state_isvalid(sef_cb_lu_state_isvalid_standard);
    /* - Register a custom routine to save the state. */
    sef_setcb_lu_state_save(sef_cb_lu_state_save);

    /* Let SEF perform startup. */
    sef_startup();
}

static int sef_cb_init(int type, sef_init_info_t *UNUSED(info)) {
/* Initialize the adler driver. */
    int do_announce_driver = TRUE;

    restore_initial_state();
    switch (type) {
        case SEF_INIT_FRESH:
            break;

        case SEF_INIT_LU:
            /* Restore the state. */
            lu_state_restore();
            do_announce_driver = FALSE;
            break;

        case SEF_INIT_RESTART:
            break;
    }

    /* Announce we are up when necessary. */
    if (do_announce_driver) {
        chardriver_announce();
    }

    /* Initialization completed successfully. */
    return OK;
}

int main(void) {
    /*
     * Perform initialization.
     */
    sef_local_startup();

    /*
     * Run the main loop.
     */
    chardriver_task(&adler_tab);
    return OK;
}
