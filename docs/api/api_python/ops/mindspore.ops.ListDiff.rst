mindspore.ops.ListDiff
======================

.. py:class:: mindspore.ops.ListDiff(out_idx=mstype.int32)

    比较两个数字列表之间的不同。

    给定一个列表 `x` 和一个列表 `y`，此操作返回一个列表 `out`，表示在 `x` 中但不在 `y` 中的所有值。
    返回列表 `out` 的排序顺序与数字出现在 `x` 中的顺序相同（保留重复项）。此操作还会返回一个列表 `idx` ，
    表示每个 `out` 元素在 `x` 中的位置。即： :math:`out[i] = x[idx[i]] for i in [0, 1, ..., len(out) - 1]` 。

    参数：
        - **out_idx** (:class:`mindspore.dtype`，可选) - `idx` 的数据类型，可选值： `mindspore.dtype.int32` 和 `mindspore.dtype.int64` 。默认值： `mindspore.dtype.int32` 。

    输入：
        - **x** (Tensor) - 一个1-D Tensor。保留的值。类型支持：[float16, float32, float64, uint8, uint16, int8, int16, int32, int64]。
        - **y** (Tensor) - 一个1-D Tensor，与 `x` 类型一致。移除的值。

    输出：
        - **out** (Tensor) - 一个1-D Tensor，与 `x` 类型一致。
        - **idx** (Tensor) - 一个1-D Tensor， `out_idx` 类型。

    异常：
        - **ValueError** - 如果 `x` 或 `y` 的shape不是1D。
        - **TypeError** - 如果 `x` 或 `y` 不是Tensor。
        - **TypeError** - 如果 `x` 或 `y` 的数据类型不在支持列表里。
        - **TypeError** - 如果 `x` 与 `y` 的数据类型不同。
        - **TypeError** - 如果属性 `out_idx` 的取值不在[mindspore.dtype.int32, mindspore.dtype.int64]中。
