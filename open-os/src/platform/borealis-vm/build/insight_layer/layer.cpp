// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <time.h>
#include <vulkan/utility/vk_dispatch_table.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan.h>

#include <cstdlib>
#include <fstream>
#include <map>
// NOLINTNEXTLINE(build/c++11)
#include <mutex>

// single global lock, for simplicity
std::mutex global_lock;
typedef std::lock_guard<std::mutex> scoped_lock;

// use the loader's dispatch table pointer as a key for dispatch map lookups
template <typename DispatchableType> void *GetKey(DispatchableType inst) {
  return static_cast<void *>(inst);
}

void log(const char *message) {
  const time_t timer = time(nullptr);
  fprintf(stderr, "%.19s INSIGHT LAYER: %s\n", ctime(&timer), message);
}

// layer book-keeping information, to store dispatch tables by key
std::map<void *, VkuInstanceDispatchTable> instance_dispatch;

static VkLayerInstanceCreateInfo *
getChainInfo(const VkInstanceCreateInfo *pCreateInfo) {
  const VkLayerInstanceCreateInfo *info =
      static_cast<const VkLayerInstanceCreateInfo *>(pCreateInfo->pNext);

  while (info &&
         (info->sType != VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO ||
          info->function != VK_LAYER_LINK_INFO)) {
    info = static_cast<const VkLayerInstanceCreateInfo *>(info->pNext);
  }

  return const_cast<VkLayerInstanceCreateInfo *>(info);
}

static PFN_vkGetInstanceProcAddr advanceChain(VkLayerInstanceCreateInfo *info) {
  assert(info->u.pLayerInfo);
  PFN_vkGetInstanceProcAddr gpa =
      info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
  info->u.pLayerInfo = info->u.pLayerInfo->pNext;

  return gpa;
}

static VkResult
VK_LAYER_insight_CreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator,
                                VkInstance *pInstance) {
  const char *pEngineName = nullptr;

  if (pCreateInfo->pApplicationInfo) {
    pEngineName = pCreateInfo->pApplicationInfo->pEngineName;
  } else {
    pEngineName = "None";
  }

  const char *path;
  if (!(path = std::getenv("INSIGHT_LAYER_ENGINE_FILE"))) {
    path = "/tmp/engine.log";
  }
  std::ofstream output(path);
  if (!output.fail()) {
    output << "Engine: " << pEngineName << std::endl;
  } else {
    char message[50];
    snprintf(message, sizeof(message),
             "Failed to write engine.log with engine: %s", pEngineName);
    log(message);
  }
  output.close();

  VkLayerInstanceCreateInfo *info = getChainInfo(pCreateInfo);
  if (info == nullptr) {
    log("Failed to get chain info");
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  PFN_vkGetInstanceProcAddr gpa = advanceChain(info);
  PFN_vkCreateInstance createInstanceFunc =
      (PFN_vkCreateInstance)gpa(VK_NULL_HANDLE, "vkCreateInstance");

  VkResult result = createInstanceFunc(pCreateInfo, pAllocator, pInstance);
  if (result != VK_SUCCESS) {
    log("Failed to create instance func");
    return result;
  }

  // fetch our own dispatch table for the functions we need, into the next layer
  VkuInstanceDispatchTable dispatchTable;
  dispatchTable.GetInstanceProcAddr =
      (PFN_vkGetInstanceProcAddr)gpa(*pInstance, "vkGetInstanceProcAddr");
  dispatchTable.DestroyInstance =
      (PFN_vkDestroyInstance)gpa(*pInstance, "vkDestroyInstance");

  // store the table by key
  {
    scoped_lock l(global_lock);
    instance_dispatch[GetKey(*pInstance)] = dispatchTable;
  }

  return result;
}

static void
VK_LAYER_insight_DestroyInstance(VkInstance instance,
                                 const VkAllocationCallbacks *pAllocator) {
  PFN_vkDestroyInstance destroyInstance;
  {
    scoped_lock l(global_lock);
    destroyInstance = instance_dispatch[GetKey(instance)].DestroyInstance;
    instance_dispatch.erase(GetKey(instance));
  }
  destroyInstance(instance, pAllocator);
}

///////////////////////////////////////////////////////////////////////////////
// GetProcAddr functions, entry points of the layer

#define GETPROCADDR(func)                                                      \
  if (strcmp(pName, "vk" #func) == 0) {                                        \
    return (PFN_vkVoidFunction) & VK_LAYER_insight_##func;                     \
  }

extern "C" PFN_vkVoidFunction VKAPI_CALL
VK_LAYER_insight_GetInstanceProcAddr(VkInstance instance, const char *pName) {
  // instance chain functions we intercept
  GETPROCADDR(GetInstanceProcAddr);
  GETPROCADDR(CreateInstance);
  GETPROCADDR(DestroyInstance);

  PFN_vkGetInstanceProcAddr gpa;
  {
    scoped_lock l(global_lock);
    gpa = instance_dispatch[GetKey(instance)].GetInstanceProcAddr;
  }
  return gpa(instance, pName);
}
