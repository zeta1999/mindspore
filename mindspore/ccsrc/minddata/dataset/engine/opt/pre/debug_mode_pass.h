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

#ifndef DATASET_ENGINE_OPT_PRE_DEBUG_MODE_PASS_H_
#define DATASET_ENGINE_OPT_PRE_DEBUG_MODE_PASS_H_

#include <memory>
#include <vector>
#include "minddata/dataset/engine/opt/pass.h"

namespace mindspore {
namespace dataset {
/// \class DebugModePass
/// \brief This is a pre parse pass that disable some nodes and prepares for the debug mode.
class DebugModePass : public IRNodePass {
 public:
  /// \brief Constructor
  DebugModePass() {}

  /// \brief Destructor
  ~DebugModePass() = default;

  /// \brief Runs a pass on MapNode
  /// \param[in] node The node being visited
  /// \param[in, out] *modified indicates if the node was changed at all
  /// \return Status code
  Status Visit(std::shared_ptr<MapNode> node, bool *const modified) override;

  /// \brief Runs a pass on DatasetNode
  /// \param[in] node The node being visited
  /// \param[in, out] *modified indicates if the node was changed at all
  /// \return Status code
  Status Visit(std::shared_ptr<DatasetNode> node, bool *const modified) override;

 protected:
  /// \brief Check and remove cache and offload if node has any
  /// \param[in] node The node being visited
  /// \return true if the node was changed; otherwise, false
  bool RemoveCacheAndOffload(std::shared_ptr<DatasetNode> node);
};
}  // namespace dataset
}  // namespace mindspore

#endif  // DATASET_ENGINE_OPT_PRE_DEBUG_MODE_PASS_H_
