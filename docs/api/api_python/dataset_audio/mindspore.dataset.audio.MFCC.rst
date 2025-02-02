mindspore.dataset.audio.MFCC
============================

.. py:class:: mindspore.dataset.audio.MFCC(sample_rate=16000, n_mfcc=40, dct_type=2, norm=NormMode.ORTHO, log_mels=False, melkwargs=None)

    计算音频信号的梅尔频率倒谱系数。

    参数：
        - **sample_rate** (int, 可选) - 采样频率（单位：Hz），不能小于零。默认值：16000。
        - **n_mfcc** (int, 可选) - 要保留的梅尔频率倒谱系数数，不能小于零。默认：40。
        - **dct_type** (int, 可选) - 要使用的离散余弦变换类型（离散余弦变换），只能为2。默认：2。
        - **norm** (NormMode, 可选) - 要使用的标准类型。默认：NormMode.ORTHO。
        - **log_mels** (bool, 可选) - 是否使用梅尔对数谱图而不是分贝刻度。默认：False。
        - **melkwargs** (dict, 可选) - 梅尔频谱的参数，如果为None则使用默认参数。默认：None，会被设置为
           `{'n_fft': 400, 'win_length': n_fft, 'hop_length': win_length // 2, 'f_min' : 0.0, 'f_max' : sample_rate // 2, 'pad': 0, 'window': WindowType.HANN, 'power': 2.0, 'normalized': False, 'center': True, 'pad_mode': BorderType.REFLECT, 'onesided': True, 'norm' : NormType.NONE, 'mel_scale' : MelType.HTK}` 。

    异常：
        - **TypeError** - 如果 `sample_rate` 的类型不为int。
        - **TypeError** - 如果 `log_mels` 的类型不为bool。
        - **TypeError** - 如果 `norm` 的类型不为 :class:`mindspore.dataset.audio.utils.NormMode` 。
        - **TypeError** - 如果 `n_mfcc` 的类型不为int。
        - **TypeError** - 如果 `melkwargs` 的类型不为dict。
        - **ValueError** - 如果 `sample_rate` 为负数。
        - **ValueError** - 如果 `n_mfcc` 为负数。
        - **ValueError** - 如果 `dct_type` 不为2。
