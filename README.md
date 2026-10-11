# Weather Data Processing & Scalability System

This project is a high-performance computational system and interactive dashboard for processing large weather datasets, extracting statistical information from historical observations, and investigating parallel scalability under increasing dataset sizes and thread configurations.

## Included files

- `index.html` — Main dashboard user interface (with execution time metrics and scalability table)
- `styles.css` — Modern dark-theme dashboard styling
- `app.js` — High-performance CSV parsing, statistical reduction, execution time modeling, and Amdahl's Law scalability calculation
- `sample-data.csv` — Standard 6-column sample weather observations
- `AWS Dataset Pangandaran.csv` — Real-world automatic weather station observation dataset (8,085 records)
- `sequential.c` — High-performance sequential C baseline
- `parallel.c` — OpenMP multi-threaded parallel C implementation

## Run the Web Dashboard locally

### Option 1: Run local HTTP server (Recommended)
From this directory, run:

```bash
python -m http.server 8000
```

Then open in your browser:
```text
http://localhost:8000
```

### Option 2: Open directly
Open `index.html` directly in any web browser.

## Features

1. **Robust Multi-Schema CSV Parser**: Automatically detects columns for temperature, humidity, pressure, rainfall, and wind speed. Works seamlessly with both standard 6-column formats and full AWS meteorological station datasets. Handles multiline headers, quoted fields, and filters out summary/footer rows without displaying `NaN`.
2. **Approximate Execution Time & Scalability Analysis**:
   - Sequential Execution Time ($T_{seq}$)
   - Parallel OpenMP Execution Time ($T_{par}$)
   - Estimated Speedup ($S = T_{seq} / T_{par}$)
   - Parallel Efficiency ($E = S / p \times 100\%$)
   - Live Client-Side JavaScript Runtime
3. **Interactive Thread Scalability Reference Table**: Automatically highlights the selected thread configuration (1–16 threads) and displays speedup, efficiency, and scaling behavior.
4. **Guaranteed Valid Numerical Displays**: Zero-division guards and strict `isFinite` sanitization ensure all numbers display accurately with exact units and never show `NaN`.

## Compile and Run C Implementations

### Sequential Version
```bash
gcc sequential.c -o sequential
./sequential "AWS Dataset Pangandaran.csv"
# or
./sequential sample-data.csv
```

### Parallel OpenMP Version
```bash
gcc -fopenmp parallel.c -o parallel
./parallel "AWS Dataset Pangandaran.csv" 4
# or
./parallel sample-data.csv 4
```
*(Replace `4` with the desired number of threads, e.g., 1, 2, 4, 8, 16).*
