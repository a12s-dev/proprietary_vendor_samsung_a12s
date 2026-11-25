/*
 * Shim: the A12 XJ5 camera HAL (libexynoscamera3.so) was built against an
 * older libui that exported:
 *     android::GraphicBufferMapper::unlock(const native_handle_t*)
 * AOSP 16 changed it to:
 *     status_t GraphicBufferMapper::unlock(buffer_handle_t, base::unique_fd* outFence)
 * Provide the old symbol and forward to the current implementation, mirroring
 * the "no out fence" path: wait for the release fence and close it.
 */
#include <sync/sync.h>
#include <ui/GraphicBufferMapper.h>

extern "C" int a12_gbm_unlock(void* self, const native_handle_t* handle)
        asm("_ZN7android19GraphicBufferMapper6unlockEPK13native_handle");

extern "C" int a12_gbm_unlock(void* self, const native_handle_t* handle) {
    android::GraphicBufferMapper* mapper =
            reinterpret_cast<android::GraphicBufferMapper*>(self);

    android::base::unique_fd fence;
    android::status_t status =
            mapper->unlock(reinterpret_cast<buffer_handle_t>(handle), &fence);

    if (fence.ok()) {
        sync_wait(fence.get(), -1);
    }

    return status;
}
