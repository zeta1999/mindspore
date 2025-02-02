mindspore.ops.csr_sqrt
=======================

.. py:function:: mindspore.ops.csr_sqrt(x: CSRTensor)

    逐元素返回当前CSRTensor的平方根。

    .. math::
        out_{i} = \\csr_sqrt{x_{i}}

    参数：
        - **x** (CSRTensor) - 输入CSRTensor，数据类型为number.Number，其rank需要在[0, 7]范围内.

    返回：
        CSRTensor，具有与 `x` 相同的shape。

    异常：
        - **TypeError** - 如果 `x` 不是CSRTensor。

