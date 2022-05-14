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

#include "plugin/device/cpu/kernel/is_close_cpu_kernel.h"
#include <cmath>
#include <algorithm>
#include <memory>
#include "abstract/utils.h"
#include "mindspore/core/ops/is_close.h"
#include "plugin/device/cpu/hal/device/cpu_device_address.h"

namespace mindspore {
namespace kernel {
namespace {
constexpr size_t kIsCloseInputsNum = 2;
constexpr size_t kIsCloseOutputsNum = 1;
template <typename T>
inline bool compute(T a, T b, float rtol, float atol, bool equal_nan) {
  if (a == b) {
    return true;
  }
  if (equal_nan && std::isnan(a) && std::isnan(b)) {
    return true;
  }
  if (atol == 0 && rtol == 0) {
    return false;
  }
  auto left_side = std::abs(a - b);
  auto right_side = atol + (rtol * std::abs(b));
  return std::isfinite(left_side) && left_side <= right_side;
}
}  // namespace

bool IsCloseCpuKernelMod::Init(const BaseOperatorPtr &base_operator, const std::vector<KernelTensorPtr> &inputs,
                               const std::vector<KernelTensorPtr> &outputs) {
  kernel_name_ = base_operator->name();
  CHECK_KERNEL_INPUTS_NUM(inputs.size(), kIsCloseInputsNum, kernel_name_);
  CHECK_KERNEL_OUTPUTS_NUM(outputs.size(), kIsCloseOutputsNum, kernel_name_);
  auto kernel_attr = GetKernelAttrFromTensors(inputs, outputs);
  auto [is_match, index] = MatchKernelAttr(kernel_attr, GetOpSupport());
  if (!is_match) {
    MS_LOG(ERROR) << "For '" << kernel_name_ << "', it does not support this kernel data type: " << kernel_attr;
    return false;
  }
  kernel_func_ = func_list_[index].second;
  auto kernel_ptr = std::make_shared<ops::IsClose>(base_operator->GetPrim());
  rtol_ = kernel_ptr->get_rtol();
  atol_ = kernel_ptr->get_atol();
  equal_nan_ = kernel_ptr->get_equal_nan();
  return true;
}

int IsCloseCpuKernelMod::Resize(const BaseOperatorPtr &base_operator, const std::vector<KernelTensorPtr> &inputs,
                                const std::vector<KernelTensorPtr> &outputs,
                                const std::map<uint32_t, tensor::TensorPtr> &) {
  if (int ret = KernelMod::Resize(base_operator, inputs, outputs) != KRET_OK) {
    return ret;
  }
  input_shape_.clear();
  other_shape_.clear();
  output_shape_.clear();
  auto input_shape = inputs.at(kIndex0)->GetShapeVector();
  (void)std::transform(input_shape.begin(), input_shape.end(), std::back_inserter(input_shape_), LongToSize);
  auto other_shape = inputs.at(kIndex1)->GetShapeVector();
  (void)std::transform(other_shape.begin(), other_shape.end(), std::back_inserter(other_shape_), LongToSize);
  auto output_shape = outputs.at(kIndex0)->GetShapeVector();
  (void)std::transform(output_shape.begin(), output_shape.end(), std::back_inserter(output_shape_), LongToSize);
  return KRET_OK;
}

template <typename T>
bool IsCloseCpuKernelMod::LaunchKernel(const std::vector<AddressPtr> &inputs,
                                       const std::vector<kernel::AddressPtr> &outputs) {
  CHECK_KERNEL_INPUTS_NUM(inputs.size(), kIsCloseInputsNum, kernel_name_);
  CHECK_KERNEL_OUTPUTS_NUM(outputs.size(), kIsCloseOutputsNum, kernel_name_);
  auto input = reinterpret_cast<T *>(inputs[kIndex0]->addr);
  auto other = reinterpret_cast<T *>(inputs[kIndex1]->addr);
  auto output = reinterpret_cast<bool *>(outputs[kIndex0]->addr);

  CTask task;
  BroadcastIterator base_iter(input_shape_, other_shape_, output_shape_);
  if (input_shape_ == other_shape_) {
    task = [this, &input, &other, &output](size_t start, size_t end) {
      for (size_t i = start; i < end; i++) {
        if constexpr (!std::is_same_v<T, float> && !std::is_same_v<T, double>) {
          auto a = static_cast<float>(input[i]);
          auto b = static_cast<float>(other[i]);
          output[i] = compute<float>(a, b, rtol_, atol_, equal_nan_);
        } else {
          output[i] = compute<T>(input[i], other[i], rtol_, atol_, equal_nan_);
        }
      }
      return common::SUCCESS;
    };
  } else {
    task = [this, &base_iter, &input, &other, &output](size_t start, size_t end) {
      auto iter = base_iter;
      iter.SetPos(start);
      for (size_t i = start; i < end; i++) {
        auto idx1 = iter.GetInputPosA();
        auto idx2 = iter.GetInputPosB();
        if constexpr (!std::is_same_v<T, float> && !std::is_same_v<T, double>) {
          auto a = static_cast<float>(input[idx1]);
          auto b = static_cast<float>(other[idx2]);
          output[i] = compute<float>(a, b, rtol_, atol_, equal_nan_);
        } else {
          output[i] = compute<T>(input[idx1], other[idx2], rtol_, atol_, equal_nan_);
        }
        iter.GenNextPos();
      }
      return common::SUCCESS;
    };
  }
  size_t elem_num = outputs[kIndex0]->size / sizeof(bool);
  ParallelLaunchAutoSearch(task, elem_num, this, &parallel_search_info_);
  return true;
}

std::vector<std::pair<KernelAttr, IsCloseCpuKernelMod::IsCloseLaunchFunc>> IsCloseCpuKernelMod::func_list_ = {
  {KernelAttr().AddInputAttr(kNumberTypeBool).AddInputAttr(kNumberTypeBool).AddOutputAttr(kNumberTypeBool),
   &IsCloseCpuKernelMod::LaunchKernel<bool>},
  {KernelAttr().AddInputAttr(kNumberTypeInt8).AddInputAttr(kNumberTypeInt8).AddOutputAttr(kNumberTypeBool),
   &IsCloseCpuKernelMod::LaunchKernel<int8_t>},
  {KernelAttr().AddInputAttr(kNumberTypeInt16).AddInputAttr(kNumberTypeInt16).AddOutputAttr(kNumberTypeBool),
   &IsCloseCpuKernelMod::LaunchKernel<int16_t>},
  {KernelAttr().AddInputAttr(kNumberTypeInt32).AddInputAttr(kNumberTypeInt32).AddOutputAttr(kNumberTypeBool),
   &IsCloseCpuKernelMod::LaunchKernel<int32_t>},
  {KernelAttr().AddInputAttr(kNumberTypeInt64).AddInputAttr(kNumberTypeInt64).AddOutputAttr(kNumberTypeBool),
   &IsCloseCpuKernelMod::LaunchKernel<int64_t>},
  {KernelAttr().AddInputAttr(kNumberTypeFloat16).AddInputAttr(kNumberTypeFloat16).AddOutputAttr(kNumberTypeBool),
   &IsCloseCpuKernelMod::LaunchKernel<float16>},
  {KernelAttr().AddInputAttr(kNumberTypeFloat32).AddInputAttr(kNumberTypeFloat32).AddOutputAttr(kNumberTypeBool),
   &IsCloseCpuKernelMod::LaunchKernel<float>},
  {KernelAttr().AddInputAttr(kNumberTypeFloat64).AddInputAttr(kNumberTypeFloat64).AddOutputAttr(kNumberTypeBool),
   &IsCloseCpuKernelMod::LaunchKernel<double>},
  {KernelAttr().AddInputAttr(kNumberTypeUInt8).AddInputAttr(kNumberTypeUInt8).AddOutputAttr(kNumberTypeBool),
   &IsCloseCpuKernelMod::LaunchKernel<uint8_t>},
  {KernelAttr().AddInputAttr(kNumberTypeUInt16).AddInputAttr(kNumberTypeUInt16).AddOutputAttr(kNumberTypeBool),
   &IsCloseCpuKernelMod::LaunchKernel<uint16_t>},
  {KernelAttr().AddInputAttr(kNumberTypeUInt32).AddInputAttr(kNumberTypeUInt32).AddOutputAttr(kNumberTypeBool),
   &IsCloseCpuKernelMod::LaunchKernel<uint32_t>},
  {KernelAttr().AddInputAttr(kNumberTypeUInt64).AddInputAttr(kNumberTypeUInt64).AddOutputAttr(kNumberTypeBool),
   &IsCloseCpuKernelMod::LaunchKernel<uint64_t>}};

std::vector<KernelAttr> IsCloseCpuKernelMod::GetOpSupport() {
  std::vector<KernelAttr> support_list;
  (void)std::transform(func_list_.begin(), func_list_.end(), std::back_inserter(support_list),
                       [](const std::pair<KernelAttr, IsCloseLaunchFunc> &pair) { return pair.first; });
  return support_list;
}

MS_KERNEL_FACTORY_REG(NativeCpuKernelMod, IsClose, IsCloseCpuKernelMod);
}  // namespace kernel
}  // namespace mindspore
