import numpy as np
from scipy.signal import firwin, remez

sample_rate = 44100  # Hz
nyquist = sample_rate / 2
lowcut = 697  # DTMF low freq
highcut = 1633  # DTMF high freq
numtaps = 7  # Filter order

# Generate the filter coefficients
coeffs = firwin(
    numtaps,
    [lowcut / nyquist, highcut / nyquist],
    pass_zero=False,
    window="hamming",
)

# Display output as a C array
print("const dtmf_float_t fir_coeffs[{}] = {{".format(numtaps))
for i in range(0, numtaps, 4):
    chunk = coeffs[i : i + 4]
    print("    " + ", ".join("{:.7f}".format(c) for c in chunk) + ",")
print("};")
