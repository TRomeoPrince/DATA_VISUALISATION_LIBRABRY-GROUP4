# Sample CSV datasets for Group 4 Graph Studio

All four datasets are fictitious practice data. They use a simple comma-separated format with a header row and no quoted/multiline fields, so they work with the current C++ DataLoader.

| Dataset | Select in Graph Studio | Best use |
| --- | --- | --- |
| `student_marks.csv` | `Mark` | Histogram (marks vs frequency) and KDE Plot (marks vs density) |
| `student_marks.csv` | `Programme` | Count Plot (programme vs number of students) |
| `subject_preferences.csv` | `PreferredSubject` | Count Plot (preferred subject vs count) |
| `subject_marks.csv` | `Subject` labels and `Mark` values | Bar Chart (subject vs actual mark) |
| `monthly_sales.csv` | `Month` labels and `Sales` values | Bar Chart (monthly sales) |

The draft currently has the three original plot types: Histogram, KDE Plot, and Count Plot. A **Bar Chart** requires the separate Bar Chart module and two-column selection UI to be implemented and merged; the two bar-chart datasets are provided ahead of that feature.

To use a CSV, open Graph Studio, click **Browse CSV**, select a file from `data/input/`, choose an appropriate graph type and column, and click **Generate SVG Graph**. SVG images are saved to `data/output/`.

The `sample.csv` file is the original small demonstration dataset and is still included.
