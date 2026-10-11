#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <time.h>

#define LINE_SIZE 2048
#define INITIAL_CAPACITY 10000

typedef struct {
    long long count;

    double temperature_sum;
    double temperature_min;
    double temperature_max;

    double humidity_sum;
    double humidity_min;
    double humidity_max;

    double pressure_sum;
    double pressure_min;
    double pressure_max;

    double rainfall_sum;
    double rainfall_min;
    double rainfall_max;

    double windspeed_sum;
    double windspeed_min;
    double windspeed_max;
} WeatherStats;


/* Initialize statistics */
void initialize_stats(WeatherStats *stats)
{
    stats->count = 0;

    stats->temperature_sum = 0.0;
    stats->temperature_min = DBL_MAX;
    stats->temperature_max = -DBL_MAX;

    stats->humidity_sum = 0.0;
    stats->humidity_min = DBL_MAX;
    stats->humidity_max = -DBL_MAX;

    stats->pressure_sum = 0.0;
    stats->pressure_min = DBL_MAX;
    stats->pressure_max = -DBL_MAX;

    stats->rainfall_sum = 0.0;
    stats->rainfall_min = DBL_MAX;
    stats->rainfall_max = -DBL_MAX;

    stats->windspeed_sum = 0.0;
    stats->windspeed_min = DBL_MAX;
    stats->windspeed_max = -DBL_MAX;
}


/* Process one weather observation */
void process_record(
    WeatherStats *stats,
    double temperature,
    double humidity,
    double pressure,
    double rainfall,
    double windspeed)
{
    stats->count++;

    /* Temperature */
    stats->temperature_sum += temperature;

    if (temperature < stats->temperature_min)
        stats->temperature_min = temperature;

    if (temperature > stats->temperature_max)
        stats->temperature_max = temperature;


    /* Humidity */
    stats->humidity_sum += humidity;

    if (humidity < stats->humidity_min)
        stats->humidity_min = humidity;

    if (humidity > stats->humidity_max)
        stats->humidity_max = humidity;


    /* Pressure */
    stats->pressure_sum += pressure;

    if (pressure < stats->pressure_min)
        stats->pressure_min = pressure;

    if (pressure > stats->pressure_max)
        stats->pressure_max = pressure;


    /* Rainfall */
    stats->rainfall_sum += rainfall;

    if (rainfall < stats->rainfall_min)
        stats->rainfall_min = rainfall;

    if (rainfall > stats->rainfall_max)
        stats->rainfall_max = rainfall;


    /* Wind Speed */
    stats->windspeed_sum += windspeed;

    if (windspeed < stats->windspeed_min)
        stats->windspeed_min = windspeed;

    if (windspeed > stats->windspeed_max)
        stats->windspeed_max = windspeed;
}


/* Display final results */
void print_results(const WeatherStats *stats, double compute_time, double total_time)
{
    if (stats->count == 0) {
        printf("No valid weather records found.\n");
        return;
    }

    printf("\n============================================\n");
    printf("       WEATHER DATA ANALYSIS\n");
    printf("       SEQUENTIAL BASELINE VERSION\n");
    printf("============================================\n");

    printf("Number of records : %lld\n\n", stats->count);

    printf("Temperature (C)\n");
    printf("  Average : %.2f\n", stats->temperature_sum / stats->count);
    printf("  Minimum : %.2f\n", stats->temperature_min);
    printf("  Maximum : %.2f\n\n", stats->temperature_max);

    printf("Humidity (%%)\n");
    printf("  Average : %.2f\n", stats->humidity_sum / stats->count);
    printf("  Minimum : %.2f\n", stats->humidity_min);
    printf("  Maximum : %.2f\n\n", stats->humidity_max);

    printf("Pressure (hPa)\n");
    printf("  Average : %.2f\n", stats->pressure_sum / stats->count);
    printf("  Minimum : %.2f\n", stats->pressure_min);
    printf("  Maximum : %.2f\n\n", stats->pressure_max);

    printf("Rainfall (mm)\n");
    printf("  Total   : %.2f\n", stats->rainfall_sum);
    printf("  Minimum : %.2f\n", stats->rainfall_min);
    printf("  Maximum : %.2f\n\n", stats->rainfall_max);

    printf("Wind Speed (km/h)\n");
    printf("  Average : %.2f\n", stats->windspeed_sum / stats->count);
    printf("  Minimum : %.2f\n", stats->windspeed_min);
    printf("  Maximum : %.2f\n\n", stats->windspeed_max);

    printf("Computation Time : %.6f seconds\n", compute_time);
    printf("Total Exec Time  : %.6f seconds (including I/O)\n", total_time);

    printf("============================================\n");
}


int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <csv_file>\n", argv[0]);
        return 1;
    }

    clock_t total_start = clock();

    FILE *file = fopen(argv[1], "r");

    if (file == NULL) {
        perror("Error opening CSV file");
        return 1;
    }

    /* Allocate dynamic arrays matching the parallel version structure */
    long long capacity = INITIAL_CAPACITY;
    long long count = 0;

    double *temperature = malloc(capacity * sizeof(double));
    double *humidity = malloc(capacity * sizeof(double));
    double *pressure = malloc(capacity * sizeof(double));
    double *rainfall = malloc(capacity * sizeof(double));
    double *windspeed = malloc(capacity * sizeof(double));

    if (!temperature || !humidity || !pressure || !rainfall || !windspeed) {
        printf("Memory allocation failed.\n");
        if (temperature) free(temperature);
        if (humidity) free(humidity);
        if (pressure) free(pressure);
        if (rainfall) free(rainfall);
        if (windspeed) free(windspeed);
        fclose(file);
        return 1;
    }

    char line[LINE_SIZE];

    /* Skip header line */
    if (fgets(line, sizeof(line), file) == NULL) {
        printf("CSV file is empty.\n");
        free(temperature); free(humidity); free(pressure); free(rainfall); free(windspeed);
        fclose(file);
        return 1;
    }

    int record_number;
    char date[50];
    char time_value[50];
    int interval;
    double temp_val, hum_val, press_val, wind_val, rain_val;
    char wind_direction[50];

    /* Sequential file reading phase */
    while (fgets(line, sizeof(line), file))
    {
        /* Format 1: 10 columns (AWS Pangandaran schema) */
        int fields = sscanf(
            line,
            " %d , %49[^,] , %49[^,] , %d , %lf , %lf , %lf , %lf , %49[^,] , %lf",
            &record_number,
            date,
            time_value,
            &interval,
            &temp_val,
            &hum_val,
            &press_val,
            &wind_val,
            wind_direction,
            &rain_val
        );

        /* Format 2: 6 columns (Standard sample format: Date,Temp,Humidity,Pressure,Rainfall,Wind) */
        if (fields != 10) {
            fields = sscanf(
                line,
                " %49[^,] , %lf , %lf , %lf , %lf , %lf",
                date,
                &temp_val,
                &hum_val,
                &press_val,
                &rain_val,
                &wind_val
            );
            if (fields == 6) {
                fields = 10; // Normalized valid match
            }
        }

        if (fields == 10)
        {
            if (count >= capacity) {
                capacity *= 2;
                double *nt = realloc(temperature, capacity * sizeof(double));
                double *nh = realloc(humidity, capacity * sizeof(double));
                double *np = realloc(pressure, capacity * sizeof(double));
                double *nr = realloc(rainfall, capacity * sizeof(double));
                double *nw = realloc(windspeed, capacity * sizeof(double));

                if (!nt || !nh || !np || !nr || !nw) {
                    printf("Memory reallocation failed.\n");
                    free(temperature); free(humidity); free(pressure); free(rainfall); free(windspeed);
                    fclose(file);
                    return 1;
                }
                temperature = nt; humidity = nh; pressure = np; rainfall = nr; windspeed = nw;
            }

            temperature[count] = temp_val;
            humidity[count] = hum_val;
            pressure[count] = press_val;
            rainfall[count] = rain_val;
            windspeed[count] = wind_val;
            count++;
        }
    }

    fclose(file);

    if (count == 0) {
        printf("No valid weather records found.\n");
        free(temperature); free(humidity); free(pressure); free(rainfall); free(windspeed);
        return 1;
    }

    WeatherStats stats;
    initialize_stats(&stats);

    /* Pure computational phase timed separately for direct speedup comparison with OpenMP */
    clock_t comp_start = clock();

    for (long long i = 0; i < count; i++) {
        process_record(
            &stats,
            temperature[i],
            humidity[i],
            pressure[i],
            rainfall[i],
            windspeed[i]
        );
    }

    clock_t comp_end = clock();
    clock_t total_end = clock();

    double compute_time = (double)(comp_end - comp_start) / CLOCKS_PER_SEC;
    double total_time = (double)(total_end - total_start) / CLOCKS_PER_SEC;

    print_results(&stats, compute_time, total_time);

    free(temperature);
    free(humidity);
    free(pressure);
    free(rainfall);
    free(windspeed);

    return 0;
}