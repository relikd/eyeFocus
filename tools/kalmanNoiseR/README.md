### Info:
Evaluate pupil-pos files and generate Kalman error matrix. Output matrix for each file and one for all files combined. Also output the maximum difference from one frame to the next (eg. max distance the pupil can travel between two frames)

Standard deviation with the following format. Where left pupil position is ( ᵪ₁ ᵧ₁ ) and right pupil is ( ᵪ₂ ᵧ₂ ). The covariance is zero except for the two y-position, since both eyes are synchronous in y direction.

|||||
|-------|-------|-------|-------|
| σᵪ₁^2 |   0   |   0   |   0   |
|   0   | σᵧ₁^2 |   0   | σᵧ₁σᵧ₂ |
|   0   |   0   | σᵪ₂^2 |   0   |
|   0   | σᵧ₂σᵧ₁ |   0   | σᵧ₂^2 |


### Usage:
> ./logEvaluator FILE.csv [FILE.csv] [...]
