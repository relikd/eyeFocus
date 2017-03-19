### Info:
Evaluate pupil-pos files and generate Kalman error matrix. Output matrix for each file and one for all files combined. 
The variance-covariance-matrix is given with the following format. Where left pupil position is ( ᵪ₁ ᵧ₁ ) and right pupil is ( ᵪ₂ ᵧ₂ ). The covariance is zero except for the two y-positions, since both eyes are synchronous in y direction.

|  pLx  |  pLy  |  pRx  |  pRy  |
|-------|-------|-------|-------|
| σᵪ₁^2 |   0   |   0   |   0   |
|   0   | σᵧ₁^2 |   0   | σᵧ₁σᵧ₂ |
|   0   |   0   | σᵪ₂^2 |   0   |
|   0   | σᵧ₂σᵧ₁ |   0   | σᵧ₂^2 |


An additional output is the maximum travel difference from one frame to the next (eg. max distance the pupil can travel between two frames). The number of combined samples and the 99-%-Quantil is written to the console. 99% of the data stays within this threshold.

A detailed output can be created with the optional parameter `-f`. Two files contain all differences sorted by amplitude. `frameDifference.txt` for the difference between frames and `meanDifference.txt` for the difference to the mean value.


### Usage:
> ./logEvaluator [-f] FILE.csv [FILE2.csv] [...]
