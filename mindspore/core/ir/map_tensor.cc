/**
 * Copyright 2022 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ir/map_tensor.h"
#include "abstract/abstract_value.h"
#include "utils/log_adapter.h"

namespace mindspore {
using tensor::Tensor;
using tensor::TensorPtr;

std::size_t MapTensor::hash() const { return static_cast<std::size_t>(tid()); }

bool MapTensor::operator==(const MapTensor &other) const { return this == &other; }

abstract::AbstractBasePtr MapTensor::ToAbstract() {
  if (param_info_ != nullptr) {
    // For parameter, a broaden abstract is created with ref_key set.
    ValuePtr ref_key = std::make_shared<RefKey>(param_info_->name());
    return std::make_shared<abstract::AbstractMapTensor>(shared_from_base<MapTensor>(), ref_key);
  } else {
    // For value, an abstract is created with value set.
    return std::make_shared<abstract::AbstractMapTensor>(shared_from_base<MapTensor>());
  }
}

TensorPtr MapTensor::Get(const TensorPtr &key_tensor, const TensorPtr &default_value) {
  MS_EXCEPTION_IF_NULL(key_tensor);
  MS_EXCEPTION_IF_NULL(default_value);
  return nullptr;
}

void MapTensor::Put(const TensorPtr &key_tensor, const TensorPtr &value_tensor) {
  MS_EXCEPTION_IF_NULL(key_tensor);
  MS_EXCEPTION_IF_NULL(value_tensor);
}

void MapTensor::Erase(const TensorPtr &key_tensor) { MS_EXCEPTION_IF_NULL(key_tensor); }

void MapTensor::Update(const MapTensor::ExportData &data) {
  MS_EXCEPTION_IF_NULL(data.key_tensor);
  MS_EXCEPTION_IF_NULL(data.value_tensor);
}

MapTensor::ExportData MapTensor::Export(bool full) {
  MS_LOG(DEBUG) << (full ? "Full" : "Incremental") << " export MapTensor";
  return {nullptr, nullptr, nullptr};
}
}  // namespace mindspore
