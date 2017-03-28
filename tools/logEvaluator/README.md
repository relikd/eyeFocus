### Info:
Loop over all files and process individual data points. A data point contains the pupil and eye corner coordinates, as well as the euclidean distance between them. Each data point corresponds to one frame image.

Write output to `meanError.csv` containing the min, max and average values for each data point property (4 points, 2 distances). Furthermore the error margin (average to lower and upper bound, min to max) is calculated.

The additional property `ratio` is created with:  
`ratio = pupil_distance / corner_distance`

Grouping is possible either by filename (focus distance) or property type (eg. ratio). This file is used for further error calculation and validity reports.


### Usage:
> ./logEvaluator [-skip 50] [-ext avi] PATH


### Note:
- `PATH` must contain files in the format: `{1-200}[c]m.{ext}.pupilpos.csv`
- optional `-e`/`-ext`: filename extension (default: MP4)
- optional `-s`/`-skip`: ignore first x values (default: 0)