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

#include "backend/common/pass/insert_type_transform_op.h"

#include <memory>
#include <vector>
#include "backend/common/session/anf_runtime_algorithm.h"
#include "include/common/utils/utils.h"
#include "include/common/utils/anfalgo.h"

namespace mindspore {
namespace opt {
int64_t SplitTupleInputs(const FuncGraphPtr &graph, const AnfNodePtr &tuple_input,
                         std::vector<AnfNodePtr> *plant_inputs) {
  MS_EXCEPTION_IF_NULL(graph);
  MS_EXCEPTION_IF_NULL(tuple_input);
  MS_EXCEPTION_IF_NULL(plant_inputs);

  if (!common::AnfAlgo::IsTupleOutput(tuple_input)) {
    auto abs = tuple_input->abstract();
    MS_EXCEPTION_IF_NULL(abs);
    MS_LOG(WARNING) << "The Function only split the output type is tuple type but got" << abs->ToString();
    return -1;
  }
  MS_EXCEPTION_IF_NULL(plant_inputs);
  auto input_size = common::AnfAlgo::GetOutputTensorNum(tuple_input);
  if (tuple_input->isa<CNode>() && common::AnfAlgo::CheckPrimitiveType(tuple_input, prim::kPrimMakeTuple)) {
    auto make_tuple = tuple_input->cast<CNodePtr>();
    MS_EXCEPTION_IF_NULL(make_tuple);
    size_t tuple_input_num = common::AnfAlgo::GetInputTensorNum(make_tuple);
    for (size_t j = 0; j < tuple_input_num; ++j) {
      // using for graph kernel
      auto dyn_input_node = common::AnfAlgo::GetInputNode(make_tuple, j);
      MS_EXCEPTION_IF_NULL(dyn_input_node);
      // Handle tuple nested scenes.
      if (dyn_input_node->isa<CNode>() && common::AnfAlgo::CheckPrimitiveType(dyn_input_node, prim::kPrimMakeTuple)) {
        input_size += SplitTupleInputs(graph, dyn_input_node, plant_inputs);
        continue;
      }
      (void)plant_inputs->emplace_back(dyn_input_node);
    }
    return input_size;
  }
  for (size_t index = 0; index < input_size; ++index) {
    auto dynamic_input_node = CreatTupleGetItemNode(graph, tuple_input, index);
    (void)plant_inputs->emplace_back(dynamic_input_node);
  }
  return input_size;
}

AnfNodePtr CreateNewNode(const FuncGraphPtr &func_graph, const AnfNodePtrList &input_list,
                         const CNodePtr &origin_node) {
  MS_EXCEPTION_IF_NULL(func_graph);
  MS_EXCEPTION_IF_NULL(origin_node);

  auto new_cnode = NewCNode(input_list, func_graph, {origin_node});
  MS_EXCEPTION_IF_NULL(new_cnode);
  // This pass should not have new node whose abstract differs from the original node. So set the original node's
  // abstract.
  new_cnode->set_abstract(origin_node->abstract());
  new_cnode->set_scope(origin_node->scope());
  new_cnode->set_primal_attrs(origin_node->primal_attrs());
  new_cnode->set_attrs(origin_node->attrs());
  auto kernel_graph = func_graph->cast<KernelGraphPtr>();
  if (kernel_graph != nullptr) {
    kernel_graph->FrontBackendlMapUpdate(origin_node, new_cnode);
  }

  // Need to reset new cnode's kernel build info because the inputs type and number could be changed after processing
  // methods. Only reset input types.
  UpdateKernelBuildInfo(new_cnode, origin_node);
  return new_cnode;
}

void UpdateKernelBuildInfo(const CNodePtr &new_cnode, const CNodePtr &origin_node) {
  MS_EXCEPTION_IF_NULL(new_cnode);
  MS_EXCEPTION_IF_NULL(origin_node);

  // Inherit from origin kernel build info.
  KernelBuildInfoPtr origin_kernel_build_info = AnfAlgo::GetSelectKernelBuildInfo(origin_node);
  auto new_kernel_builder = std::make_shared<kernel::KernelBuildInfo::KernelBuildInfoBuilder>(origin_kernel_build_info);

  // Construct new inputs info and set to the new kernel build info.
  std::vector<std::string> inputs_device_format;
  std::vector<TypeId> inputs_device_type;
  size_t input_num = common::AnfAlgo::GetInputTensorNum(new_cnode);
  for (size_t input_index = 0; input_index < input_num; ++input_index) {
    inputs_device_format.push_back(AnfAlgo::GetOutputFormat(new_cnode->input(input_index + kSizeOne), kIndex0));
    inputs_device_type.push_back(AnfAlgo::GetOutputDeviceDataType(new_cnode->input(input_index + kSizeOne), kIndex0));
  }
  new_kernel_builder->SetInputsFormat(inputs_device_format);
  new_kernel_builder->SetInputsDeviceType(inputs_device_type);

  auto kernel_info = std::make_shared<device::KernelInfo>();
  MS_EXCEPTION_IF_NULL(kernel_info);
  new_cnode->set_kernel_info(kernel_info);
  AnfAlgo::SetSelectKernelBuildInfo(new_kernel_builder->Build(), new_cnode.get());
}

// A map of kernel object type pairs to processing functions.
static std::map<ObjectTypePair, ProcessTypeTransformFunc> kTypePairToProcessFunc;

InsertTypeTransformOp::InsertTypeTransformOp(bool multigraph)
    : PatternProcessPass("insert_type_transform_op", multigraph) {
  kTypePairToProcessFunc[{KernelObjectType::TUPLE_UNFOLD, KernelObjectType::TUPLE_UNFOLD}] =
    std::bind(&InsertTypeTransformOp::ProcessTupleUnfoldToTupleUnfold, this, std::placeholders::_1,
              std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
}

const AnfNodePtr InsertTypeTransformOp::Process(const FuncGraphPtr &func_graph, const AnfNodePtr &node,
                                                const EquivPtr &) const {
  MS_EXCEPTION_IF_NULL(func_graph);
  MS_EXCEPTION_IF_NULL(node);
  if (!node->isa<CNode>() || !AnfUtils::IsRealKernel(node)) {
    return nullptr;
  }

  if (common::AnfAlgo::CheckPrimitiveType(node, prim::kPrimCall) ||
      common::AnfAlgo::CheckPrimitiveType(node, prim::kPrimPartial)) {
    return nullptr;
  }

  auto needed_input_type_list = AnfAlgo::GetInputKernelObjectTypes(node);
  auto cnode = node->cast<CNodePtr>();
  MS_EXCEPTION_IF_NULL(cnode);
  AnfNodePtrList new_input_list = {common::AnfAlgo::GetCNodePrimitiveNode(cnode)};
  for (size_t i = kIndex1; i < cnode->inputs().size(); i++) {
    MS_LOG(DEBUG) << "Kernel object type of index " << i << " is "
                  << kObjectTypeToString[needed_input_type_list[i - 1]];
    // Get actually needed input kernel object type.
    KernelObjectType needed_input_type = needed_input_type_list[i - 1];

    // Get current input's kernel object type.
    auto current_input_type_list = AnfAlgo::GetOutputKernelObjectTypes(cnode->inputs()[i]);
    KernelObjectType current_input_type = current_input_type_list[kIndex0];

    ObjectTypePair type_pair = {current_input_type, needed_input_type};
    if (kTypePairToProcessFunc.count(type_pair) != 0) {
      MS_LOG(INFO) << "Kernel object type pair of input index " << (i - 1) << " for node "
                   << cnode->fullname_with_scope() << " is " << type_pair.to_string();
      bool new_prim = false;
      AnfNodePtrList processed_input_list =
        kTypePairToProcessFunc[type_pair](func_graph, cnode->inputs()[i], cnode, &new_prim);
      if (new_prim) {
        // If new primitive is created, replace the old one, which is the first element of the input list.
        new_input_list[kIndex0] = processed_input_list[kIndex0];
        // Jump the primitive node the first one, and the rest is the new inputs.
        new_input_list.insert(new_input_list.end(), std::begin(processed_input_list) + kIndex1,
                              processed_input_list.end());
      } else {
        new_input_list.insert(new_input_list.end(), processed_input_list.begin(), processed_input_list.end());
      }
    } else {
      // If this input type is valid, just push back the origin input.
      new_input_list.push_back(cnode->inputs()[i]);
    }
  }

  // Create replacing node, update front-end node map, set kernel build info, inherit attributes, etc. These operations
  // could rely on the origin CNode.
  auto new_node = CreateNewNode(func_graph, new_input_list, cnode);
  return new_node;
}

AnfNodePtrList InsertTypeTransformOp::ProcessTupleUnfoldToTupleUnfold(const FuncGraphPtr &func_graph,
                                                                      const AnfNodePtr &input, const CNodePtr &node,
                                                                      bool *new_prim) {
  MS_EXCEPTION_IF_NULL(input);
  MS_EXCEPTION_IF_NULL(node);

  // If the input needs to be skipped as ConvertTupleInputToDynamicInput does, return the input node itself for caller
  // to construct input list.
  bool is_bprop_cut = common::AnfAlgo::CheckPrimitiveType(node, prim::kPrimBpropCut);
  bool skip = is_bprop_cut && input->abstract()->isa<abstract::AbstractSparseTensor>();
  if (skip) {
    return {input};
  }

  AnfNodePtrList plant_inputs;
  int64_t unfold_num = SplitTupleInputs(func_graph, input, &plant_inputs);
  MS_LOG(DEBUG) << "Tuple unfold input: " << input->fullname_with_scope() << " has " << unfold_num << " outputs.";
  return plant_inputs;
}
}  // namespace opt
}  // namespace mindspore
