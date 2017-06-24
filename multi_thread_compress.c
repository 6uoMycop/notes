
static void *do_data_compress(void *opaque)
{
    CompressParam *param = opaque;

    while (!quit_comp_thread) {
        qemu_mutex_lock(&param->mutex);
        /* Re-check the quit_comp_thread in case of
         * terminate_compression_threads is called just before
         * qemu_mutex_lock(&param->mutex) and after
         * while(!quit_comp_thread), re-check it here can make
         * sure the compression thread terminate as expected.
         */
        while (!param->start && !quit_comp_thread) {
            qemu_cond_wait(&param->cond, &param->mutex);
        }
        if (!quit_comp_thread) {
            /* do compress according to param->block and param->offset */
            do_compress_ram_page(param);
        }
        param->start = false;
        qemu_mutex_unlock(&param->mutex);

        qemu_mutex_lock(comp_done_lock);
        param->done = true;
        qemu_cond_signal(comp_done_cond);
        qemu_mutex_unlock(comp_done_lock);
    }

    return NULL;
}

void migrate_compress_threads_create(void)
{
    int i, thread_count;

    if (!migrate_use_compression()) {
        return;
    }
    quit_comp_thread = false;
    compression_switch = true;
    thread_count = migrate_compress_threads();
    compress_threads = g_new0(QemuThread, thread_count);
    comp_param = g_new0(CompressParam, thread_count);
    comp_done_cond = g_new0(QemuCond, 1);
    comp_done_lock = g_new0(QemuMutex, 1);
    qemu_cond_init(comp_done_cond);
    qemu_mutex_init(comp_done_lock);
    for (i = 0; i < thread_count; i++) {
        /* com_param[i].file is just used as a dummy buffer to save data, set
         * it's ops to empty.
         */
        comp_param[i].file = qemu_fopen_ops(NULL, &empty_ops);
        comp_param[i].done = true;
        qemu_mutex_init(&comp_param[i].mutex);
        qemu_cond_init(&comp_param[i].cond);
        qemu_thread_create(compress_threads + i, "compress",
                           do_data_compress, comp_param + i,
                           QEMU_THREAD_JOINABLE);
    }
}

static inline void start_compression(CompressParam *param)
{
    param->done = false;
    qemu_mutex_lock(&param->mutex);
    param->start = true;
    qemu_cond_signal(&param->cond);
    qemu_mutex_unlock(&param->mutex);
}

static void flush_compressed_data(QEMUFile *f)
{
    int idx, len, thread_count;

    if (!migrate_use_compression()) {
        return;
    }
    thread_count = migrate_compress_threads();
    for (idx = 0; idx < thread_count; idx++) {
        if (!comp_param[idx].done) {
            qemu_mutex_lock(comp_done_lock);
            while (!comp_param[idx].done && !quit_comp_thread) {
                qemu_cond_wait(comp_done_cond, comp_done_lock);
            }
            qemu_mutex_unlock(comp_done_lock);
        }
        if (!quit_comp_thread) {
            // flush data
            len = qemu_put_qemu_file(f, comp_param[idx].file);
            bytes_transferred += len;
        }
    }
}

static inline void set_compress_params(CompressParam *param, RAMBlock *block,
                                       ram_addr_t offset)
{
    param->block = block;
    param->offset = offset;
}

static int compress_page_with_multi_thread(QEMUFile *f, RAMBlock *block,
                                           ram_addr_t offset,
                                           uint64_t *bytes_transferred)
{
    int idx, thread_count, bytes_xmit = -1, pages = -1;

    thread_count = migrate_compress_threads();
    qemu_mutex_lock(comp_done_lock);
    while (true) {
        /* find an available thread to compress this page */
        for (idx = 0; idx < thread_count; idx++) {
            if (comp_param[idx].done) {
                bytes_xmit = qemu_put_qemu_file(f, comp_param[idx].file);
                set_compress_params(&comp_param[idx], block, offset);
                start_compression(&comp_param[idx]);
                pages = 1;
                acct_info.norm_pages++;
                *bytes_transferred += bytes_xmit;
                break;
            }
        }
        if (pages > 0) {
            break;
        } else {
            /* no available thread */
            qemu_cond_wait(comp_done_cond, comp_done_lock);
        }
    }
    qemu_mutex_unlock(comp_done_lock);

    return pages;
}