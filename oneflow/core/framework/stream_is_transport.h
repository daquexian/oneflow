/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef ONEFLOW_CORE_FRAMEWORK_STREAM_IS_TRANSPORT_H_
#define ONEFLOW_CORE_FRAMEWORK_STREAM_IS_TRANSPORT_H_

#include <glog/logging.h>
#include "oneflow/core/common/stream_role.h"

namespace oneflow {

struct StreamIsTransport {
  static bool Case(SR<StreamRole::kInvalid>) {  // NOLINT
    LOG(FATAL);
  }
  static bool Case(SR<StreamRole::kCompute>) { return false; }
  static bool Case(SR<StreamRole::kHost2Device>) { return false; }
  static bool Case(SR<StreamRole::kDevice2Host>) { return false; }
  static bool Case(SR<StreamRole::kSyncedLaunchedCC>) { return true; }
  static bool Case(SR<StreamRole::kAsyncedLaunchedCC>) { return true; }
  static bool Case(SR<StreamRole::kCriticalSection>) { return false; }
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_FRAMEWORK_STREAM_IS_TRANSPORT_H_
