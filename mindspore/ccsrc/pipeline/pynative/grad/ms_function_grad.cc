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

#include "pipeline/pynative/grad/ms_function_grad.h"
#include "pipeline/pynative/pynative_utils.h"
#include "include/common/utils/anfalgo.h"
#include "include/common/utils/parallel_context.h"
#include "ir/func_graph_cloner.h"

namespace mindspore {
namespace pynative {
namespace {
FrontendOpRunInfoPtr GetOpRunInfo(const py::object &out, const py::args &args, const GradExecutor *grad_executor,
                                  const std::string &graph_phase, ValuePtr *added_out_v) {
  // Get actual output value and added output value.
  if (!py::isinstance<py::tuple>(out)) {
    MS_LOG(EXCEPTION) << "The output value of ms_function func graph should be a tuple.";
  }
  auto tuple_out = py::cast<py::tuple>(out);
  constexpr size_t tuple_out_size = 2;
  if (tuple_out.size() != tuple_out_size) {
    MS_LOG(EXCEPTION) << "The tuple size of output value of ms_function func graph should be 2.";
  }
  FrontendOpRunInfoPtr op_run_info = std::make_shared<FrontendOpRunInfo>();
  // Output of ms_function
  op_run_info->out_value = PyNativeAlgo::DataConvert::PyObjToValue(tuple_out[0]);
  MS_EXCEPTION_IF_NULL(added_out_v);
  // Forward output of op in ms_function graph
  *added_out_v = PyNativeAlgo::DataConvert::PyObjToValue(tuple_out[1]);
  MS_LOG(DEBUG) << "Added output value is: " << (*added_out_v)->ToString();

  op_run_info->base_op_run_info.op_name = graph_phase;
  MS_EXCEPTION_IF_NULL(grad_executor);
  op_run_info->base_op_run_info.abstract = op_run_info->out_value->ToAbstract();
  size_t input_args_size = args.size();
  for (size_t i = 0; i < input_args_size; ++i) {
    (void)op_run_info->input_value.emplace_back(PyNativeAlgo::DataConvert::PyObjToValue(args[i]));
  }
  return op_run_info;
}

size_t GetOutputTensorNumForTuple(const CNodePtr &make_tuple) {
  size_t output_num = 0;
  MS_EXCEPTION_IF_NULL(make_tuple);
  if (IsPrimitiveCNode(make_tuple, prim::kPrimMakeTuple)) {
    for (size_t i = 1; i < make_tuple->size(); ++i) {
      const auto &input_i = make_tuple->input(i);
      MS_EXCEPTION_IF_NULL(input_i);
      if (input_i->isa<CNode>()) {
        auto cnode = input_i->cast<CNodePtr>();
        MS_EXCEPTION_IF_NULL(cnode);
        output_num += GetOutputTensorNumForTuple(cnode);
      } else if (input_i->isa<Parameter>()) {
        output_num += 1;
      }
    }
  } else {
    output_num += common::AnfAlgo::GetOutputTensorNum(make_tuple);
  }
  return output_num;
}
}  // namespace

void MsFunction::RunReplace(const CNodePtr &added_make_tuple,
                            const std::vector<tensor::TensorPtr> &total_output_tensors,
                            const FuncGraphPtr &grad_graph) const {
  MS_EXCEPTION_IF_NULL(grad_graph);
  MS_EXCEPTION_IF_NULL(added_make_tuple);
  size_t index = 0;
  for (size_t i = 1; i < added_make_tuple->size(); ++i) {
    const auto &input_i = added_make_tuple->input(i);
    MS_EXCEPTION_IF_NULL(input_i);
    auto cnode = input_i->cast<CNodePtr>();
    MS_EXCEPTION_IF_NULL(cnode);
    MS_LOG(DEBUG) << "Replace new output tensors for cnode: " << cnode->DebugString();
    auto output_vnode = cnode->forward().first;
    MS_EXCEPTION_IF_NULL(output_vnode);
    // To clean up all value nodes in PyNative after run grad graph
    grad_graph->AddValueNode(output_vnode);
    MS_LOG(DEBUG) << "Original output value node: " << output_vnode->ToString();
    size_t output_num = GetOutputTensorNumForTuple(cnode);
    if (index + output_num > total_output_tensors.size()) {
      MS_LOG(EXCEPTION) << "The size of total_output_tensors: " << total_output_tensors.size()
                        << ", but the current index: " << index << ", output num: " << output_num;
    }
    // Get new tensors.
    std::vector<ValuePtr> new_values;
    for (size_t j = index; j < index + output_num; ++j) {
      (void)new_values.emplace_back(total_output_tensors[j]);
    }
    index = index + output_num;
    // Replace new tensors.
    if (output_num == 1) {
      output_vnode->set_value(new_values[0]);
    } else if (output_num > 1) {
      output_vnode->set_value(std::make_shared<ValueTuple>(new_values));
    } else {
      MS_LOG(EXCEPTION) << "The output value of forward cnode is empty, forward cnode info: " << cnode->ToString();
    }
    MS_LOG(DEBUG) << "New output value node: " << output_vnode->ToString();
  }
  // Save op info with new tensors for current running ms_function func graph.
  if (index != total_output_tensors.size()) {
    MS_LOG(EXCEPTION) << "The index: " << index
                      << " should be equal to the size of total_output_tensors: " << total_output_tensors.size();
  }
}

void MsFunction::ReplaceNewTensorsInGradGraph(const TopCellInfoPtr &top_cell, const string &op_info,
                                              const ValuePtr &added_out, const FuncGraphPtr &ms_func_graph,
                                              const FuncGraphPtr &grad_graph) const {
  MS_EXCEPTION_IF_NULL(top_cell);
  MS_EXCEPTION_IF_NULL(added_out);
  MS_EXCEPTION_IF_NULL(ms_func_graph);
  // Get added forward nodes.
  auto merge_node = ms_func_graph->output();
  MS_EXCEPTION_IF_NULL(merge_node);
  auto merge_make_tuple = merge_node->cast<CNodePtr>();
  MS_EXCEPTION_IF_NULL(merge_make_tuple);
  constexpr size_t merge_output_size = 3;
  // First is make_tuple, second is actual output, third is added output
  if (merge_make_tuple->size() != merge_output_size) {
    MS_LOG(EXCEPTION) << "The input size of merge make tuple node should be 3, but it is: " << merge_make_tuple->size();
  }
  constexpr size_t added_output_index = 2;
  const auto &added_forward_node = merge_make_tuple->input(added_output_index);
  MS_EXCEPTION_IF_NULL(added_forward_node);
  // Just one added output
  if (added_forward_node->isa<ValueNode>()) {
    MS_LOG(DEBUG) << "The added forward output node is value node: " << added_forward_node->DebugString();
    std::vector<tensor::TensorPtr> total_output_tensors;
    TensorValueToTensor(added_out, &total_output_tensors);
    top_cell->set_op_info_with_ms_func_forward_tensors(op_info, total_output_tensors);
    return;
  }
  // Replace new output tensors for forward nodes, it will also work in grad graph with same value node.
  auto added_make_tuple = added_forward_node->cast<CNodePtr>();
  MS_EXCEPTION_IF_NULL(added_make_tuple);
  MS_LOG(DEBUG) << "The added forward make tuple node info: " << added_make_tuple->DebugString();
  std::vector<tensor::TensorPtr> total_output_tensors;
  TensorValueToTensor(added_out, &total_output_tensors);
  MS_EXCEPTION_IF_NULL(grad_graph);
  // The forward node in ms_function graph is created during compilation and is a
  // placeholder(mindspore/ccsrc/frontend/optimizer/ad/pynative_dfunctor.cc).After running ms_function, need to update
  // to real value.
  RunReplace(added_make_tuple, total_output_tensors, grad_graph);
  (void)std::for_each(total_output_tensors.begin(), total_output_tensors.end(),
                      [](const tensor::TensorPtr &tensor) { tensor->set_is_forward_output(true); });
  top_cell->set_op_info_with_ms_func_forward_tensors(op_info, total_output_tensors);
}

void MsFunction::UpdateMsFunctionForwardTensors(const GradExecutor *grad_executor, const string &op_info,
                                                const ValuePtr &new_forward_value) const {
  MS_EXCEPTION_IF_NULL(new_forward_value);
  MS_LOG(DEBUG) << "Ms func graph has already ran before. The graph phase is: " << graph_phase_;
  MS_LOG(DEBUG) << "The output values of added forward nodes are: " << new_forward_value->ToString();
  std::vector<tensor::TensorPtr> new_tensors;
  TensorValueToTensor(new_forward_value, &new_tensors);
  if (new_tensors.empty()) {
    MS_LOG(DEBUG) << "The size of added forward tensors is zero, no need to update.";
    return;
  }
  MS_EXCEPTION_IF_NULL(grad_executor);
  const auto &top_cell = grad_executor->top_cell();
  const auto &old_tensors = top_cell->op_info_with_ms_func_forward_tensors().at(op_info);
  if (old_tensors.size() != new_tensors.size()) {
    MS_LOG(EXCEPTION) << "The size of old tensors is: " << old_tensors.size()
                      << ", but the size of new tensors is: " << new_tensors.size()
                      << ", the current op info is: " << op_info;
  }
  for (size_t i = 0; i < new_tensors.size(); ++i) {
    grad_executor->UpdateTensorInfo(new_tensors[i], {old_tensors[i]});
    old_tensors[i]->set_sync_status(kNeedSyncDeviceToHost);
  }
}

void MsFunction::GetInputArgsNode(const FrontendOpRunInfoPtr &op_run_info, AnfNodePtrList *input_nodes,
                                  const GradExecutor *grad_executor) const {
  MS_EXCEPTION_IF_NULL(op_run_info);
  MS_EXCEPTION_IF_NULL(input_nodes);
  MS_EXCEPTION_IF_NULL(grad_executor);
  size_t input_args_size = op_run_info->input_value.size();
  for (size_t i = 0; i < input_args_size; ++i) {
    const auto &input_i_value = op_run_info->input_value[i];
    MS_LOG(DEBUG) << "The input " << i << " value of ms_function graph is: " << input_i_value->ToString();
    const auto &input_i_node = grad_executor->GetInput(input_i_value);
    MS_EXCEPTION_IF_NULL(input_i_node);
    MS_LOG(DEBUG) << "The input " << i << " node of ms_function graph is: " << input_i_node->DebugString();
    (void)input_nodes->emplace_back(input_i_node);
  }
}

void MsFunction::GetWeightsNode(const FrontendOpRunInfoPtr &op_run_info, const GradExecutor *grad_executor,
                                const FuncGraphPtr &ms_func_graph, AnfNodePtrList *input_nodes) const {
  MS_EXCEPTION_IF_NULL(grad_executor);
  MS_EXCEPTION_IF_NULL(input_nodes);
  const auto &top_cell = grad_executor->top_cell();
  auto df_builder = top_cell->df_builder();
  MS_EXCEPTION_IF_NULL(df_builder);
  const auto &graph_info = top_cell->graph_info_map().at(df_builder);
  MS_EXCEPTION_IF_NULL(graph_info);
  // Get weights info of ms_function
  auto manage = Manage(ms_func_graph, false);
  MS_EXCEPTION_IF_NULL(ms_func_graph);
  const auto &original_params = ms_func_graph->parameters();
  size_t params_size = original_params.size();
  std::vector<AnfNodePtr> new_params;
  MS_EXCEPTION_IF_NULL(op_run_info);
  size_t input_args_index = op_run_info->input_value.size();
  for (size_t i = 0; i < params_size; ++i) {
    if (i < input_args_index) {  // non-weights node.
      (void)new_params.emplace_back(original_params[i]);
      continue;
    }
    const auto &anf_node = original_params[i];
    auto param = anf_node->cast<ParameterPtr>();
    MS_EXCEPTION_IF_NULL(param);
    auto param_info = param->param_info();
    MS_EXCEPTION_IF_NULL(param_info);
    auto param_name = param_info->name();
    if (graph_info->params.count(param_name) != 0) {
      // Share same weight parameter in different ms_function call.
      const auto &same_param = graph_info->params.at(param_name);
      (void)manage->Replace(anf_node, same_param);
      param = same_param;
    } else {
      df_builder->add_parameter(param);
      param->debug_info()->set_name(param_name);
    }
    (void)new_params.emplace_back(param);
    (void)input_nodes->emplace_back(param);
    const auto &default_param = param->default_param();
    MS_EXCEPTION_IF_NULL(default_param);
    (void)op_run_info->input_value.emplace_back(default_param);
    top_cell->SetParamNodeMapInGraphInfoMap(df_builder, param_name, param);
    MS_LOG(DEBUG) << "Top graph set free parameter " << param->DebugString() << ". Its default value is "
                  << default_param->ToString() << ". Its name is: " << param_name;
  }
  ms_func_graph->set_parameters(new_params);
  manage->Clear();
}

void MsFunction::MakeCNodeForMsFunction(const FrontendOpRunInfoPtr &op_run_info, const GradExecutor *grad_executor,
                                        const FuncGraphPtr &ms_func_graph, CNodePtr *ms_function_cnode) const {
  // Get input node info of ms_function
  std::vector<AnfNodePtr> input_nodes{NewValueNode(ms_func_graph)};
  MS_EXCEPTION_IF_NULL(grad_executor);
  GetInputArgsNode(op_run_info, &input_nodes, grad_executor);
  // Get weights node info of ms_function.
  GetWeightsNode(op_run_info, grad_executor, ms_func_graph, &input_nodes);
  // Make a CNode which includes ms_function fprop graph and inputs node
  MS_EXCEPTION_IF_NULL(ms_function_cnode);
  *ms_function_cnode = grad_executor->top_cell()->fg()->NewCNode(input_nodes);
  MS_LOG(DEBUG) << "Make ms function forward CNode: " << (*ms_function_cnode)->DebugString();
}

// Make adjoint for ms_function fprop graph and connect it with previous op
CNodePtr MsFunction::MakeAdjointForMsFunction(const FrontendOpRunInfoPtr &op_run_info,
                                              const GradExecutor *grad_executor, const FuncGraphPtr &ms_func_graph,
                                              const FuncGraphPtr &grad_graph) const {
  MS_EXCEPTION_IF_NULL(op_run_info);
  MS_EXCEPTION_IF_NULL(grad_executor);
  CNodePtr ms_function_cnode = nullptr;
  MakeCNodeForMsFunction(op_run_info, grad_executor, ms_func_graph, &ms_function_cnode);
  MS_EXCEPTION_IF_NULL(ms_function_cnode);
  const auto &top_cell = grad_executor->top_cell();
  const auto &out_id = PyNativeAlgo::Common::GetIdByValue(op_run_info->out_value);
  top_cell->SetTupleArgsToGraphInfoMap(top_cell->fg(), op_run_info->out_value, ms_function_cnode);
  top_cell->SetNodeMapInGraphInfoMap(top_cell->fg(), out_id, ms_function_cnode);

  // Connect grad graph of ms_function to context.
  auto k_pynative_cell_ptr = top_cell->k_pynative_cell_ptr();
  MS_EXCEPTION_IF_NULL(k_pynative_cell_ptr);
  if (!k_pynative_cell_ptr->KPynativeWithFProp(ms_function_cnode, op_run_info->input_value, op_run_info->out_value,
                                               grad_graph)) {
    MS_LOG(EXCEPTION) << "Failed to make adjoint for ms_function cnode, ms_function cnode info: "
                      << ms_function_cnode->DebugString();
  }
  top_cell->set_ms_function_flag(true);
  return ms_function_cnode;
}

void MsFunction::GradMsFunctionInner(const FrontendOpRunInfoPtr &op_run_info, const GradExecutor *grad_executor,
                                     const ValuePtr &added_out_v, const FuncGraphPtr &ms_func_graph,
                                     const FuncGraphPtr &grad_graph) const {
  MS_EXCEPTION_IF_NULL(op_run_info);
  MS_EXCEPTION_IF_NULL(grad_executor);
  MS_EXCEPTION_IF_NULL(added_out_v);
  const auto &top_cell = grad_executor->top_cell();
  top_cell->RecordGradOpInfo(op_run_info);

  // Step 1: Update actual output tensors used in grad graph.
  MS_EXCEPTION_IF_NULL(op_run_info->out_value);
  MS_LOG(DEBUG) << "ms_function actual output value: " << op_run_info->out_value->ToString();
  // The output of ms_function may be used in subsequent PyNative process
  grad_executor->UpdateForwardTensorInfoInBpropGraph(op_run_info->op_info, op_run_info->out_value);

  // Step 2: Update output tensors of added forward nodes, which are added to return node of ms_function func graph.
  if (top_cell->op_info_with_ms_func_forward_tensors().find(op_run_info->op_info) !=
      top_cell->op_info_with_ms_func_forward_tensors().end()) {
    UpdateMsFunctionForwardTensors(grad_executor, op_run_info->op_info, added_out_v);
    return;
  }
  MS_LOG(DEBUG) << "Ms func graph run firstly. The graph phase is: " << graph_phase_;
  if (!grad_executor->need_construct_graph()) {
    MS_LOG(EXCEPTION) << "The flag of need construct graph is False.";
  }
  ReplaceNewTensorsInGradGraph(top_cell, op_run_info->op_info, added_out_v, ms_func_graph, grad_graph);

  // Clone new ms_function func graph and grad graph.
  auto new_ms_func_graph = BasicClone(ms_func_graph);
  auto new_grad_graph = BasicClone(grad_graph, true);
  auto new_make_tuple = new_ms_func_graph->output()->cast<CNodePtr>();
  MS_EXCEPTION_IF_NULL(new_make_tuple);
  new_ms_func_graph->set_output(new_make_tuple->input(1));

  // Make Adjoint for grad graph
  const auto &ms_function_cnode =
    MakeAdjointForMsFunction(op_run_info, grad_executor, new_ms_func_graph, new_grad_graph);
  ms_function_cnode->set_abstract(new_ms_func_graph->output()->abstract()->Broaden());
}

void MsFunction::SetMsFuncGraphParameters(const FuncGraphPtr &ms_func_graph) {
  MS_EXCEPTION_IF_NULL(ms_func_graph);
  auto parallel_mode = parallel::ParallelContext::GetInstance()->parallel_mode();
  if (parallel_mode == parallel::kSemiAutoParallel || parallel_mode == parallel::kAutoParallel) {
    for (auto &parameter : ms_func_graph->parameters()) {
      auto param = parameter->cast<ParameterPtr>();
      if (param->has_default()) {
        (void)ms_function_params_.emplace_back(param->name());
      }
    }
  }
}

py::object MsFunction::GradMsFunction(const py::object &out, const py::args &args) {
  if (graph_phase_.empty()) {
    MS_LOG(EXCEPTION) << "The graph phase is empty, can not obtain ms_function func graph.";
  }
  // Get forward graph
  MS_LOG(DEBUG) << "ms_function func graph phase: " << graph_phase_;
  auto executor = pipeline::GraphExecutorPy::GetInstance();
  MS_EXCEPTION_IF_NULL(executor);
  FuncGraphPtr ms_func_graph = executor->GetFuncGraph(graph_phase_);
  MS_EXCEPTION_IF_NULL(ms_func_graph);
  // Get actual forward output object.
  py::object ret = out;
  if (ms_func_graph->modify_output()) {
    auto tuple_out = py::cast<py::tuple>(out);
    ret = tuple_out[0];
  }
  // Save dynamic shape info if output tensors of forward graph have dynamic shapes
  const auto &grad_executor = PyNativeAlgo::Common::GetPyNativeExecutor()->grad_executor();
  grad_executor->dynamic_shape()->SaveDynShapeAbsForMsFunction(args, out, ms_func_graph);
  // Make Adjoint for grad graph of ms_function.
  if (!grad_executor->grad_flag()) {
    MS_LOG(DEBUG) << "Only run forward infer computation, no need to construct grad graph.";
    graph_phase_.clear();
    return ret;
  }
  ValuePtr added_out_v;
  const auto &op_run_info = GetOpRunInfo(out, args, grad_executor.get(), graph_phase_, &added_out_v);
  FuncGraphPtr grad_graph = executor->GetGradGraph(graph_phase_);
  MS_EXCEPTION_IF_NULL(grad_graph);
  GradMsFunctionInner(op_run_info, grad_executor.get(), added_out_v, ms_func_graph, grad_graph);
  SetMsFuncGraphParameters(ms_func_graph);
  graph_phase_.clear();
  return ret;
}
}  // namespace pynative
}  // namespace mindspore
