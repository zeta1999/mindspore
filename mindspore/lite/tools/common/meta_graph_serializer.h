/**
 * Copyright 2021 Huawei Technologies Co., Ltd
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

#ifndef MINDSPORE_LITE_TOOLS_COMMON_META_GRAPH_SERIALIZER_H
#define MINDSPORE_LITE_TOOLS_COMMON_META_GRAPH_SERIALIZER_H

#include <fstream>
#include <string>
#include "flatbuffers/flatbuffers.h"
#include "schema/inner/model_generated.h"

namespace mindspore::lite {
class MetaGraphSerializer {
 public:
  static int Save(const schema::MetaGraphT &graph, const std::string &output_path);

 private:
  MetaGraphSerializer() = default;

  virtual ~MetaGraphSerializer() = default;

  void InitPath(const std::string &output_path);

  bool Init(const schema::MetaGraphT &graph, const std::string &output_path);

  schema::ExternalDataT *AddExternalData(std::fstream *file, const char *data, size_t size);

  bool ExtraAndSerializeModelWeight(const schema::MetaGraphT &graph);

  bool SerializeModelAndUpdateWeight(const schema::MetaGraphT &meta_graphT);

 private:
  int64_t cur_offset_ = 0;
  std::string save_path_;
  std::string model_name_;
  std::string save_model_path_;
  std::string save_data_path_;
};
}  // namespace mindspore::lite

#endif  // MINDSPORE_LITE_TOOLS_COMMON_META_GRAPH_SERIALIZER_H
