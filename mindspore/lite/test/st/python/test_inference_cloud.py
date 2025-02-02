# Copyright 2022 Huawei Technologies Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ============================================================================
"""
Test lite python API.
"""
import sys
import mindspore_lite as mslite
import numpy as np


def model_common_predict(context, model_path, in_data_path, input_shapes):
    model = mslite.Model()
    model.build_from_file(model_path, mslite.ModelType.MINDIR, context)
    inputs = model.get_inputs()
    outputs = model.get_outputs()
    for i in range(len(inputs)):
        in_data = np.fromfile(in_data_path[i], dtype=np.float32)
        inputs[i].set_shape(input_shapes[i])
        inputs[i].set_data_from_numpy(in_data)
    model.predict(inputs, outputs)


def runner_common_predict(runner_config, model_path, in_data_path, input_shapes):
    runner = mslite.ModelParallelRunner()
    runner.init(model_path=model_path, runner_config=runner_config)
    inputs = runner.get_inputs()
    for i in range(len(inputs)):
        in_data = np.fromfile(in_data_path[i], dtype=np.float32)
        inputs[i].set_shape(input_shapes[i])
        inputs[i].set_data_from_numpy(in_data)
        outputs = []
    runner.predict(inputs, outputs)


# ============================ cpu inference ============================
def test_model_inference_cpu(model_path, in_data_path, input_shapes):
    cpu_device_info = mslite.CPUDeviceInfo(enable_fp16=False)
    context = mslite.Context(thread_num=2, thread_affinity_mode=0)
    context.append_device_info(cpu_device_info)
    model_common_predict(context, model_path, in_data_path, input_shapes)


# ============================ gpu inference ============================
def test_model_inference_gpu(model_path, in_data_path, input_shapes):
    gpu_device_info = mslite.GPUDeviceInfo(device_id=0, enable_fp16=False)
    cpu_device_info = mslite.CPUDeviceInfo(enable_fp16=False)
    context = mslite.Context(thread_num=2, thread_affinity_mode=0)
    context.append_device_info(gpu_device_info)
    context.append_device_info(cpu_device_info)
    model_common_predict(context, model_path, in_data_path, input_shapes)


# ============================ ascend inference ============================
def test_model_inference_ascend(model_path, in_data_path, input_shapes):
    ascend_device_info = mslite.AscendDeviceInfo(device_id=0)
    cpu_device_info = mslite.CPUDeviceInfo(enable_fp16=False)
    context = mslite.Context(thread_num=2, thread_affinity_mode=0)
    context.append_device_info(ascend_device_info)
    context.append_device_info(cpu_device_info)
    model_common_predict(context, model_path, in_data_path, input_shapes)


# ============================ cpu server inference ============================
def test_parallel_inference_cpu(model_path, in_data_path, input_shapes):
    cpu_device_info = mslite.CPUDeviceInfo(enable_fp16=False)
    context = mslite.Context(thread_num=2, thread_affinity_mode=0)
    context.append_device_info(cpu_device_info)
    runner_config = mslite.RunnerConfig(context, 2)
    runner_common_predict(runner_config, model_path, in_data_path, input_shapes)


# ============================ gpu server inference ============================
def test_parallel_inference_gpu(model_path, in_data_path, input_shapes):
    gpu_device_info = mslite.GPUDeviceInfo(device_id=0, enable_fp16=False)
    cpu_device_info = mslite.CPUDeviceInfo(enable_fp16=False)
    context = mslite.Context(thread_num=2, thread_affinity_mode=0)
    context.append_device_info(gpu_device_info)
    context.append_device_info(cpu_device_info)
    runner_config = mslite.RunnerConfig(context, 2)
    runner_common_predict(runner_config, model_path, in_data_path, input_shapes)


# ============================ ascend server inference ============================
def test_parallel_inference_ascend(model_path, in_data_path, input_shapes):
    ascend_device_info = mslite.AscendDeviceInfo(device_id=0)
    cpu_device_info = mslite.CPUDeviceInfo(enable_fp16=False)
    context = mslite.Context(thread_num=2, thread_affinity_mode=0)
    context.append_device_info(ascend_device_info)
    context.append_device_info(cpu_device_info)
    runner_config = mslite.RunnerConfig(context, 2)
    runner_common_predict(runner_config, model_path, in_data_path, input_shapes)


if __name__ == '__main__':
    model_file = sys.argv[1]
    in_data_file = sys.argv[2]
    input_shapes_str = sys.argv[3]
    backend = sys.argv[4]
    shapes = []
    input_shapes_list = input_shapes_str.split(":")
    for input_dims in input_shapes_list:
        dim = []
        dims = input_dims.split(",")
        for d in dims:
            dim.append(int(d))
        shapes.append(dim)
    in_data_file_list = in_data_file.split(",")
    if backend == "CPU":
        test_model_inference_cpu(model_file, in_data_file_list, shapes)
        test_parallel_inference_cpu(model_file, in_data_file_list, shapes)
    elif backend == "GPU":
        test_model_inference_gpu(model_file, in_data_file_list, shapes)
        test_parallel_inference_gpu(model_file, in_data_file_list, shapes)
    elif backend == "Ascend":
        test_model_inference_ascend(model_file, in_data_file_list, shapes)
        test_parallel_inference_ascend(model_file, in_data_file_list, shapes)
    else:
        raise RuntimeError('not support backend!')
    print("run success.")
