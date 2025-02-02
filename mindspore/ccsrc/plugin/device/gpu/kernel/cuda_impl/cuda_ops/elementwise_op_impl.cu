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

#include "plugin/device/gpu/kernel/cuda_impl/cuda_ops/elementwise_op_impl.cuh"
#include <stdint.h>
#include <limits>
#include "include/cuda_fp16.h"
#include "plugin/device/cpu/kernel/nnacl/op_base.h"
#include "plugin/device/gpu/kernel/cuda_impl/cuda_ops/elementswise_op_impl.cuh"
#include "plugin/device/gpu/kernel/cuda_impl/cuda_ops/util.cuh"

// exp
template <typename T>
struct ExpFunctor {
  ExpFunctor() {}
  __device__ __forceinline__ T operator()(T x) const { return expf(x); }
};

template <>
struct ExpFunctor<double> {
  ExpFunctor() {}
  __device__ __forceinline__ double operator()(double x) const { return exp(x); }
};

template <>
struct ExpFunctor<half> {
  ExpFunctor() {}
  __device__ __forceinline__ half operator()(half x) const { return hexp(x); }
};

template <>
struct ExpFunctor<Complex<float>> {
  ExpFunctor() {}
  __device__ __forceinline__ Complex<float> operator()(Complex<float> x) const { return exp(x); }
};

template <>
struct ExpFunctor<Complex<double>> {
  ExpFunctor() {}
  __device__ __forceinline__ Complex<double> operator()(Complex<double> x) const { return exp(x); }
};

template <typename T>
void ExpOpt(const T *input, T *output, const size_t count, cudaStream_t cuda_stream) {
  ExpFunctor<T> functor;
  cuda::elementwise::Unary(functor, (uint)(count), output, input, cuda_stream);
  return;
}

template CUDA_LIB_EXPORT void ExpOpt<double>(const double *input, double *output, const size_t count,
                                             cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ExpOpt<float>(const float *input, float *output, const size_t count,
                                            cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ExpOpt<half>(const half *input, half *output, const size_t count,
                                           cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ExpOpt<char>(const char *input, char *output, const size_t count,
                                           cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ExpOpt<unsigned char>(const unsigned char *input, unsigned char *output,
                                                    const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ExpOpt<int16_t>(const int16_t *input, int16_t *output, const size_t count,
                                              cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ExpOpt<uint16_t>(const uint16_t *input, uint16_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ExpOpt<int>(const int *input, int *output, const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ExpOpt<uint32_t>(const uint32_t *input, uint32_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ExpOpt<int64_t>(const int64_t *input, int64_t *output, const size_t count,
                                              cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ExpOpt<uint64_t>(const uint64_t *input, uint64_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ExpOpt<Complex<float>>(const Complex<float> *input, Complex<float> *output,
                                                     const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ExpOpt<Complex<double>>(const Complex<double> *input, Complex<double> *output,
                                                      const size_t count, cudaStream_t cuda_stream);

// log
template <typename T>
struct LogFunctor {
  LogFunctor() {}
  __device__ __forceinline__ T operator()(T x) const { return logf(x); }
};

template <>
struct LogFunctor<double> {
  LogFunctor() {}
  __device__ __forceinline__ double operator()(double x) const { return log(x); }
};

template <>
struct LogFunctor<half> {
  LogFunctor() {}
  __device__ __forceinline__ half operator()(half x) const { return hlog(x); }
};

template <>
struct LogFunctor<Complex<float>> {
  LogFunctor() {}
  __device__ __forceinline__ Complex<float> operator()(Complex<float> x) const { return log(x); }
};

template <>
struct LogFunctor<Complex<double>> {
  LogFunctor() {}
  __device__ __forceinline__ Complex<double> operator()(Complex<double> x) const { return log(x); }
};

template <typename T>
void LogOpt(const T *input, T *output, const size_t count, cudaStream_t cuda_stream) {
  LogFunctor<T> functor;
  cuda::elementwise::Unary(functor, (uint)(count), output, input, cuda_stream);
  return;
}

template CUDA_LIB_EXPORT void LogOpt<double>(const double *input, double *output, const size_t count,
                                             cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void LogOpt<float>(const float *input, float *output, const size_t count,
                                            cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void LogOpt<half>(const half *input, half *output, const size_t count,
                                           cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void LogOpt<unsigned char>(const unsigned char *input, unsigned char *output,
                                                    const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void LogOpt<char>(const char *input, char *output, const size_t count,
                                           cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void LogOpt<int16_t>(const int16_t *input, int16_t *output, const size_t count,
                                              cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void LogOpt<uint16_t>(const uint16_t *input, uint16_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void LogOpt<int>(const int *input, int *output, const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void LogOpt<uint32_t>(const uint32_t *input, uint32_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void LogOpt<int64_t>(const int64_t *input, int64_t *output, const size_t count,
                                              cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void LogOpt<uint64_t>(const uint64_t *input, uint64_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void LogOpt<Complex<float>>(const Complex<float> *input, Complex<float> *output,
                                                     const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void LogOpt<Complex<double>>(const Complex<double> *input, Complex<double> *output,
                                                      const size_t count, cudaStream_t cuda_stream);

// neg
template <typename T>
struct NegFunctor {
  NegFunctor() {}
  __device__ __forceinline__ T operator()(T x) const { return T(-1) * x; }
};

template <typename T>
void NegOpt(const T *input, T *output, const size_t count, cudaStream_t cuda_stream) {
  NegFunctor<T> functor;
  cuda::elementwise::Unary(functor, (uint)(count), output, input, cuda_stream);
}

template CUDA_LIB_EXPORT void NegOpt<double>(const double *input, double *output, const size_t count,
                                             cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void NegOpt<float>(const float *input, float *output, const size_t count,
                                            cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void NegOpt<half>(const half *input, half *output, const size_t count,
                                           cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void NegOpt<char>(const char *input, char *output, const size_t count,
                                           cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void NegOpt<unsigned char>(const unsigned char *input, unsigned char *output,
                                                    const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void NegOpt<int16_t>(const int16_t *input, int16_t *output, const size_t count,
                                              cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void NegOpt<uint16_t>(const uint16_t *input, uint16_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void NegOpt<int>(const int *input, int *output, const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void NegOpt<uint32_t>(const uint32_t *input, uint32_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void NegOpt<int64_t>(const int64_t *input, int64_t *output, const size_t count,
                                              cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void NegOpt<uint64_t>(const uint64_t *input, uint64_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void NegOpt<Complex<float>>(const Complex<float> *input, Complex<float> *output,
                                                     const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void NegOpt<Complex<double>>(const Complex<double> *input, Complex<double> *output,
                                                      const size_t count, cudaStream_t cuda_stream);

// reciprocal
template <typename T>
struct ReciprocalFunctor {
  T zero_ = static_cast<T>(0.0);
  T one_ = static_cast<T>(1.0);
  bool has_infinity_ = false;
  ReciprocalFunctor() : has_infinity_(std::numeric_limits<T>::infinity()) {}
  __device__ __forceinline__ T operator()(T x) const {
    if (x != zero_) {
      return one_ / x;
    }
    if (has_infinity_) {
      // Referring to the execution result of numpy, We need to add 1 to positive infinity.
      return std::numeric_limits<T>::infinity() + one_;
    }
    // Referring to the execution result of numpy, We need to add 1 to positive max.
    return std::numeric_limits<T>::max() + one_;
  }
};

template <typename T>
void ReciprocalOpt(const T *input, T *output, const size_t count, cudaStream_t cuda_stream) {
  ReciprocalFunctor<T> functor;
  cuda::elementwise::Unary(functor, (uint)(count), output, input, cuda_stream);
}

template CUDA_LIB_EXPORT void ReciprocalOpt<double>(const double *input, double *output, const size_t count,
                                                    cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ReciprocalOpt<float>(const float *input, float *output, const size_t count,
                                                   cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ReciprocalOpt<half>(const half *input, half *output, const size_t count,
                                                  cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ReciprocalOpt<char>(const char *input, char *output, const size_t count,
                                                  cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ReciprocalOpt<unsigned char>(const unsigned char *input, unsigned char *output,
                                                           const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ReciprocalOpt<int16_t>(const int16_t *input, int16_t *output, const size_t count,
                                                     cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ReciprocalOpt<uint16_t>(const uint16_t *input, uint16_t *output, const size_t count,
                                                      cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ReciprocalOpt<int>(const int *input, int *output, const size_t count,
                                                 cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ReciprocalOpt<uint32_t>(const uint32_t *input, uint32_t *output, const size_t count,
                                                      cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ReciprocalOpt<int64_t>(const int64_t *input, int64_t *output, const size_t count,
                                                     cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ReciprocalOpt<uint64_t>(const uint64_t *input, uint64_t *output, const size_t count,
                                                      cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ReciprocalOpt<Complex<float>>(const Complex<float> *input, Complex<float> *output,
                                                            const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void ReciprocalOpt<Complex<double>>(const Complex<double> *input, Complex<double> *output,
                                                             const size_t count, cudaStream_t cuda_stream);

// square
template <typename T>
struct SquareFunctor {
  SquareFunctor() {}
  __device__ __forceinline__ T operator()(T x) const { return x * x; }
};

template <typename T>
void SquareOpt(const T *input, T *output, const size_t count, cudaStream_t cuda_stream) {
  SquareFunctor<T> functor;
  cuda::elementwise::Unary(functor, (uint)(count), output, input, cuda_stream);
}

template CUDA_LIB_EXPORT void SquareOpt<double>(const double *input, double *output, const size_t count,
                                                cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SquareOpt<float>(const float *input, float *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SquareOpt<half>(const half *input, half *output, const size_t count,
                                              cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SquareOpt<char>(const char *input, char *output, const size_t count,
                                              cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SquareOpt<unsigned char>(const unsigned char *input, unsigned char *output,
                                                       const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SquareOpt<int16_t>(const int16_t *input, int16_t *output, const size_t count,
                                                 cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SquareOpt<uint16_t>(const uint16_t *input, uint16_t *output, const size_t count,
                                                  cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SquareOpt<int>(const int *input, int *output, const size_t count,
                                             cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SquareOpt<uint32_t>(const uint32_t *input, uint32_t *output, const size_t count,
                                                  cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SquareOpt<int64_t>(const int64_t *input, int64_t *output, const size_t count,
                                                 cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SquareOpt<uint64_t>(const uint64_t *input, uint64_t *output, const size_t count,
                                                  cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SquareOpt<Complex<float>>(const Complex<float> *input, Complex<float> *output,
                                                        const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SquareOpt<Complex<double>>(const Complex<double> *input, Complex<double> *output,
                                                         const size_t count, cudaStream_t cuda_stream);

// sqrt
template <typename T>
struct SqrtFunctor {
  SqrtFunctor() {}
  __device__ __forceinline__ T operator()(T x) const { return sqrtf(x); }
};

template <>
struct SqrtFunctor<double> {
  SqrtFunctor() {}
  __device__ __forceinline__ double operator()(double x) const { return sqrt(x); }
};

template <>
struct SqrtFunctor<half> {
  SqrtFunctor() {}
  __device__ __forceinline__ half operator()(half x) const { return hsqrt(x); }
};

template <>
struct SqrtFunctor<Complex<float>> {
  SqrtFunctor() {}
  __device__ __forceinline__ Complex<float> operator()(Complex<float> x) const { return sqrt(x); }
};

template <>
struct SqrtFunctor<Complex<double>> {
  SqrtFunctor() {}
  __device__ __forceinline__ Complex<double> operator()(Complex<double> x) const { return sqrt(x); }
};

template <typename T>
void SqrtOpt(const T *input, T *output, const size_t count, cudaStream_t cuda_stream) {
  SqrtFunctor<T> functor;
  cuda::elementwise::Unary(functor, (uint)(count), output, input, cuda_stream);
  return;
}

template CUDA_LIB_EXPORT void SqrtOpt<double>(const double *input, double *output, const size_t count,
                                              cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SqrtOpt<float>(const float *input, float *output, const size_t count,
                                             cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SqrtOpt<half>(const half *input, half *output, const size_t count,
                                            cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SqrtOpt<char>(const char *input, char *output, const size_t count,
                                            cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SqrtOpt<unsigned char>(const unsigned char *input, unsigned char *output,
                                                     const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SqrtOpt<int16_t>(const int16_t *input, int16_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SqrtOpt<uint16_t>(const uint16_t *input, uint16_t *output, const size_t count,
                                                cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SqrtOpt<int>(const int *input, int *output, const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SqrtOpt<uint32_t>(const uint32_t *input, uint32_t *output, const size_t count,
                                                cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SqrtOpt<int64_t>(const int64_t *input, int64_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SqrtOpt<uint64_t>(const uint64_t *input, uint64_t *output, const size_t count,
                                                cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SqrtOpt<Complex<float>>(const Complex<float> *input, Complex<float> *output,
                                                      const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SqrtOpt<Complex<double>>(const Complex<double> *input, Complex<double> *output,
                                                       const size_t count, cudaStream_t cuda_stream);

// onesLike
template <typename T>
struct OnesLikeFunctor {
  T one_ = static_cast<T>(1.0);
  OnesLikeFunctor() {}
  __device__ __forceinline__ T operator()(T x) const { return one_; }
};

template <>
struct OnesLikeFunctor<half> {
  half one_ = half(1.0);
  OnesLikeFunctor() {}
  __device__ __forceinline__ half operator()(half x) const { return one_; }
};

template <typename T>
void CalOnesLike(const T *input, T *output, const size_t count, cudaStream_t cuda_stream) {
  OnesLikeFunctor<T> functor;
  cuda::elementwise::Unary(functor, (uint)(count), output, input, cuda_stream);
}

template CUDA_LIB_EXPORT void CalOnesLike<bool>(const bool *input, bool *output, const size_t count,
                                                cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalOnesLike<double>(const double *input, double *output, const size_t count,
                                                  cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalOnesLike<float>(const float *input, float *output, const size_t count,
                                                 cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalOnesLike<half>(const half *input, half *output, const size_t count,
                                                cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalOnesLike<int8_t>(const int8_t *input, int8_t *output, const size_t count,
                                                  cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalOnesLike<uint8_t>(const uint8_t *input, uint8_t *output, const size_t count,
                                                   cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalOnesLike<int16_t>(const int16_t *input, int16_t *output, const size_t count,
                                                   cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalOnesLike<uint16_t>(const uint16_t *input, uint16_t *output, const size_t count,
                                                    cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalOnesLike<int32_t>(const int32_t *input, int32_t *output, const size_t count,
                                                   cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalOnesLike<uint32_t>(const uint32_t *input, uint32_t *output, const size_t count,
                                                    cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalOnesLike<int64_t>(const int64_t *input, int64_t *output, const size_t count,
                                                   cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalOnesLike<uint64_t>(const uint64_t *input, uint64_t *output, const size_t count,
                                                    cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalOnesLike<Complex<float>>(const Complex<float> *input, Complex<float> *output,
                                                          const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalOnesLike<Complex<double>>(const Complex<double> *input, Complex<double> *output,
                                                           const size_t count, cudaStream_t cuda_stream);

// LogicalNot
template <typename T>
struct LogicalNotFunctor {
  T zero_ = static_cast<T>(0);
  LogicalNotFunctor() {}
  __device__ __forceinline__ T operator()(T x) const { return static_cast<T>(x == zero_); }
};

template <>
struct LogicalNotFunctor<bool> {
  LogicalNotFunctor() {}
  __device__ __forceinline__ bool operator()(bool x) const { return !x; }
};

template <typename T>
void LogicalNot(const T *input, T *output, const size_t count, cudaStream_t cuda_stream) {
  LogicalNotFunctor<T> functor;
  cuda::elementwise::Unary(functor, (uint)(count), output, input, cuda_stream);
}

template CUDA_LIB_EXPORT void LogicalNot<bool>(const bool *input, bool *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void LogicalNot<int>(const int *input, int *output, const size_t count,
                                              cudaStream_t cuda_stream);

// Select
template <typename T>
struct SelectFunctor {
  SelectFunctor() {}
  __device__ __forceinline__ T operator()(bool cond, T x, T y) const { return cond ? x : y; }
};

template <typename T>
void CalSelect(const bool *cond, const T *input_x, const T *input_y, T *output, const size_t count,
               cudaStream_t cuda_stream) {
  SelectFunctor<T> functor;
  cuda::elementwise::Ternary(functor, (uint)(count), output, cond, input_x, input_y, cuda_stream);
}

template CUDA_LIB_EXPORT void CalSelect<double>(const bool *cond, const double *input_x, const double *input_y,
                                                double *output, const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalSelect<float>(const bool *cond, const float *input_x, const float *input_y,
                                               float *output, const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalSelect<int>(const bool *cond, const int *input_x, const int *input_y, int *output,
                                             const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalSelect<half>(const bool *cond, const half *input_x, const half *input_y, half *output,
                                              const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalSelect<int64_t>(const bool *cond, const int64_t *input_x, const int64_t *input_y,
                                                 int64_t *output, const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalSelect<bool>(const bool *cond, const bool *input_x, const bool *input_y, bool *output,
                                              const size_t count, cudaStream_t cuda_stream);

// ReLU
template <typename T>
struct ReLUFunctor {
  T zero_ = static_cast<T>(0.0);
  ReLUFunctor() {}
  __device__ __forceinline__ T operator()(T x) const {
    return isnan(static_cast<double>(x)) || (x > zero_) ? x : zero_;
  }
};

template <>
struct ReLUFunctor<half> {
  half zero_ = half(0.0);
  ReLUFunctor() {}
  __device__ __forceinline__ half operator()(half x) const {
    return isnan(static_cast<double>(__half2float(x))) || (x > zero_) ? x : zero_;
  }
};

template <typename T>
void CalReLU(const T *input, T *output, const size_t count, cudaStream_t cuda_stream) {
  ReLUFunctor<T> functor;
  cuda::elementwise::Unary(functor, (uint)(count), output, input, cuda_stream);
}

template CUDA_LIB_EXPORT void CalReLU<double>(const double *input, double *output, const size_t count,
                                              cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalReLU<float>(const float *input, float *output, const size_t count,
                                             cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalReLU<half>(const half *input, half *output, const size_t count,
                                            cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalReLU<int8_t>(const int8_t *input, int8_t *output, const size_t count,
                                              cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalReLU<uint8_t>(const uint8_t *input, uint8_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalReLU<int16_t>(const int16_t *input, int16_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalReLU<int32_t>(const int32_t *input, int32_t *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void CalReLU<int64_t>(const int64_t *input, int64_t *output, const size_t count,
                                               cudaStream_t cuda_stream);

// tanh
template <typename T>
struct TanhFunctor {
  TanhFunctor() {}
  __device__ __forceinline__ T operator()(T x) const {
    float input_x = static_cast<float>(x);
    return static_cast<T>(tanhf(input_x));
  }
};

template <>
struct TanhFunctor<double> {
  TanhFunctor() {}
  __device__ __forceinline__ double operator()(double x) const { return tanh(x); }
};

template <>
struct TanhFunctor<float> {
  TanhFunctor() {}
  __device__ __forceinline__ float operator()(float x) const { return tanhf(x); }
};

template <>
struct TanhFunctor<half> {
  TanhFunctor() {}
  __device__ __forceinline__ half operator()(half x) const {
    // e^x - e^-x / e^x + e^-x
    half pos = hexp(x);
    half neg = hexp(-x);
    return (pos - neg) / (pos + neg);
  }
};

template <>
struct TanhFunctor<Complex<float>> {
  TanhFunctor() {}
  __device__ __forceinline__ Complex<float> operator()(Complex<float> x) const { return tanh(x); }
};

template <>
struct TanhFunctor<Complex<double>> {
  TanhFunctor() {}
  __device__ __forceinline__ Complex<double> operator()(Complex<double> x) const { return tanh(x); }
};

template <typename T>
void TanhOpt(const T *input, T *output, const size_t count, cudaStream_t cuda_stream) {
  TanhFunctor<T> functor;
  cuda::elementwise::Unary(functor, (uint)(count), output, input, cuda_stream);
  return;
}

template CUDA_LIB_EXPORT void TanhOpt<double>(const double *input, double *output, const size_t count,
                                              cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void TanhOpt<float>(const float *input, float *output, const size_t count,
                                             cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void TanhOpt<half>(const half *input, half *output, const size_t count,
                                            cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void TanhOpt<Complex<float>>(const Complex<float> *input, Complex<float> *output,
                                                      const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void TanhOpt<Complex<double>>(const Complex<double> *input, Complex<double> *output,
                                                       const size_t count, cudaStream_t cuda_stream);

// sigmoid
template <typename T>
struct SigmoidFunctor {
  T one_ = static_cast<T>(1.0);
  SigmoidFunctor() {}
  __device__ __forceinline__ T operator()(T x) const { return one_ / (one_ + exp(-x)); }
};

template <>
struct SigmoidFunctor<half> {
  half one_ = half(1.0);
  SigmoidFunctor() {}
  __device__ __forceinline__ half operator()(half x) const { return one_ / (one_ + hexp(-x)); }
};

template <>
struct SigmoidFunctor<Complex<float>> {
  Complex<float> one_{1.0, 0};
  SigmoidFunctor() {}
  __device__ __forceinline__ Complex<float> operator()(Complex<float> x) const { return one_ / (one_ + exp(-x)); }
};

template <>
struct SigmoidFunctor<Complex<double>> {
  Complex<double> one_{1.0, 0};
  SigmoidFunctor() {}
  __device__ __forceinline__ Complex<double> operator()(Complex<double> x) const { return one_ / (one_ + exp(-x)); }
};

template <typename T>
void SigmoidOpt(const T *input, T *output, const size_t count, cudaStream_t cuda_stream) {
  SigmoidFunctor<T> functor;
  cuda::elementwise::Unary(functor, (uint)(count), output, input, cuda_stream);
  return;
}

template CUDA_LIB_EXPORT void SigmoidOpt<double>(const double *input, double *output, const size_t count,
                                                 cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SigmoidOpt<float>(const float *input, float *output, const size_t count,
                                                cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SigmoidOpt<half>(const half *input, half *output, const size_t count,
                                               cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SigmoidOpt<Complex<float>>(const Complex<float> *input, Complex<float> *output,
                                                         const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SigmoidOpt<Complex<double>>(const Complex<double> *input, Complex<double> *output,
                                                          const size_t count, cudaStream_t cuda_stream);

// TanhGrad
template <typename T>
struct TanhGradFunctor {
  T one_ = static_cast<T>(1.0);
  TanhGradFunctor() {}
  __device__ __forceinline__ T operator()(T y, T dout) const {
    T divisor = one_ - y * y;
    return dout * divisor;
  }
};

template <typename T>
void TanhGradOpt(const T *input, const T *dout, T *output, const size_t count, cudaStream_t cuda_stream) {
  TanhGradFunctor<T> functor;
  cuda::elementwise::Binary(functor, (uint)(count), output, input, dout, cuda_stream);
  return;
}

template CUDA_LIB_EXPORT void TanhGradOpt<double>(const double *input, const double *dout, double *output,
                                                  const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void TanhGradOpt<float>(const float *input, const float *dout, float *output,
                                                 const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void TanhGradOpt<half>(const half *input, const half *dout, half *output, const size_t count,
                                                cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void TanhGradOpt<Complex<float>>(const Complex<float> *input, const Complex<float> *dout,
                                                          Complex<float> *output, const size_t count,
                                                          cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void TanhGradOpt<Complex<double>>(const Complex<double> *input, const Complex<double> *dout,
                                                           Complex<double> *output, const size_t count,
                                                           cudaStream_t cuda_stream);

// SigmoidGrad
template <typename T>
struct SigmoidGradFunctor {
  T one_ = static_cast<T>(1.0);
  SigmoidGradFunctor() {}
  __device__ __forceinline__ T operator()(T y, T dout) const {
    T divisor = y * (one_ - y);
    return dout * divisor;
  }
};

template <>
struct SigmoidGradFunctor<Complex<float>> {
  Complex<float> one_{1.0, 0};
  SigmoidGradFunctor() {}
  __device__ __forceinline__ Complex<float> operator()(Complex<float> y, Complex<float> dout) const {
    Complex<float> divisor = y * (one_ - y);
    return dout * conj(divisor);
  }
};

template <>
struct SigmoidGradFunctor<Complex<double>> {
  Complex<double> one_{1.0, 0};
  SigmoidGradFunctor() {}
  __device__ __forceinline__ Complex<double> operator()(Complex<double> y, Complex<double> dout) const {
    Complex<double> divisor = y * (one_ - y);
    return dout * conj(divisor);
  }
};

template <typename T>
void SigmoidGradOpt(const T *input, const T *dout, T *output, const size_t count, cudaStream_t cuda_stream) {
  SigmoidGradFunctor<T> functor;
  cuda::elementwise::Binary(functor, (uint)(count), output, input, dout, cuda_stream);
  return;
}

template CUDA_LIB_EXPORT void SigmoidGradOpt<double>(const double *input, const double *dout, double *output,
                                                     const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SigmoidGradOpt<float>(const float *input, const float *dout, float *output,
                                                    const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SigmoidGradOpt<half>(const half *input, const half *dout, half *output,
                                                   const size_t count, cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SigmoidGradOpt<Complex<float>>(const Complex<float> *input, const Complex<float> *dout,
                                                             Complex<float> *output, const size_t count,
                                                             cudaStream_t cuda_stream);
template CUDA_LIB_EXPORT void SigmoidGradOpt<Complex<double>>(const Complex<double> *input, const Complex<double> *dout,
                                                              Complex<double> *output, const size_t count,
                                                              cudaStream_t cuda_stream);
