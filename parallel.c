#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <omp.h>

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


/* Process one record */
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


/* Combine statistics from one thread into another */
void combine_stats(
    WeatherStats *destination,
    const WeatherStats *source)
{
    destination->count += source->count;

    destination->temperature_sum += source->temperature_sum;

    if (source->temperature_min < destination->temperature_min)
        destination->temperature_min = source->temperature_min;

    if (source->temperature_max > destination->temperature_max)
        destination->temperature_max = source->temperature_max;


    destination->humidity_sum += source->humidity_sum;

    if (source->humidity_min < destination->humidity_min)
        destination->humidity_min = source->humidity_min;

    if (source->humidity_max > destination->humidity_max)
        destination->humidity_max = source->humidity_max;


    destination->pressure_sum += source->pressure_sum;

    if (source->pressure_min < destination->pressure_min)
        destination->pressure_min = source->pressure_min;

    if (source->pressure_max > destination->pressure_max)
        destination->pressure_max = source->pressure_max;


    destination->rainfall_sum += source->rainfall_sum;

    if (source->rainfall_min < destination->rainfall_min)
        destination->rainfall_min = source->rainfall_min;

    if (source->rainfall_max > destination->rainfall_max)
        destination->rainfall_max = source->rainfall_max;


    destination->windspeed_sum += source->windspeed_sum;

    if (source->windspeed_min < destination->windspeed_min)
        destination->windspeed_min = source->windspeed_min;

    if (source->windspeed_max > destination->windspeed_max)
        destination->windspeed_max = source->windspeed_max;
}


/* Print final results */
void print_results(
    const WeatherStats *stats,
    double execution_time,
    int num_threads)
{
    if (stats->count == 0) {
        printf("No valid weather records found.\n");
        return;
    }

    printf("\n============================================\n");
    printf("       WEATHER DATA ANALYSIS\n");
    printf("       OPENMP PARALLEL VERSION\n");
    printf("============================================\n");

    printf("Number of records : %lld\n", stats->count);
    printf("Number of threads : %d\n\n", num_threads);

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

    printf("Execution Time : %.6f seconds\n", execution_time);

    printf("============================================\n");
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <csv_file> <number_of_threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[2]);

    if (num_threads <= 0) {
        printf("Number of threads must be greater than 0.\n");
        return 1;
    }


    FILE *file = fopen(argv[1], "r");

    if (file == NULL) {
        perror("Error opening CSV file");
        return 1;
    }


    /*
       Dynamic arrays for the five numerical parameters.

       The CSV contains 8085 actual weather observations.
       We dynamically increase the capacity so the same
       program can also process larger datasets.
    */

    long long capacity = INITIAL_CAPACITY;
    long long count = 0;

    double *temperature =
        malloc(capacity * sizeof(double));

    double *humidity =
        malloc(capacity * sizeof(double));

    double *pressure =
        malloc(capacity * sizeof(double));

    double *rainfall =
        malloc(capacity * sizeof(double));

    double *windspeed =
        malloc(capacity * sizeof(double));


    if (temperature == NULL ||
        humidity == NULL ||
        pressure == NULL ||
        rainfall == NULL ||
        windspeed == NULL)
    {
        printf("Memory allocation failed.\n");

        free(temperature);
        free(humidity);
        free(pressure);
        free(rainfall);
        free(windspeed);

        fclose(file);

        return 1;
    }


    char line[LINE_SIZE];

    /* Skip header */
    if (fgets(line, sizeof(line), file) == NULL)
    {
        printf("CSV file is empty.\n");

        free(temperature);
        free(humidity);
        free(pressure);
        free(rainfall);
        free(windspeed);

        fclose(file);

        return 1;
    }


    int record_number;
    char date[50];
    char time_value[50];
    int interval;

    double temp_value;
    double humidity_value;
    double pressure_value;
    double windspeed_value;
    double rainfall_value;

    char wind_direction[50];


    /*
       Sequential input phase.
       We read the CSV once and store only the numerical
       weather parameters required for computation.
    */

    while (fgets(line, sizeof(line), file))
    {
        int fields = sscanf(
            line,
            " %d , %49[^,] , %49[^,] , %d , %lf , %lf , %lf , %lf , %49[^,] , %lf",
            &record_number,
            date,
            time_value,
            &interval,
            &temp_value,
            &humidity_value,
            &pressure_value,
            &windspeed_value,
            wind_direction,
            &rainfall_value
        );

        /* Format 2: 6 columns (Standard sample format: Date,Temp,Humidity,Pressure,Rainfall,Wind) */
        if (fields != 10)
        {
            fields = sscanf(
                line,
                " %49[^,] , %lf , %lf , %lf , %lf , %lf",
                date,
                &temp_value,
                &humidity_value,
                &pressure_value,
                &rainfall_value,
                &windspeed_value
            );
            if (fields == 6)
            {
                fields = 10; // Normalized valid match
            }
        }


        /*
           Only rows having all 10 required first columns
           are accepted.

           This automatically ignores the MIN/MAX/AVERAGE
           summary rows at the end of the dataset.
        */

        if (fields == 10)
        {
            if (count >= capacity)
            {
                capacity *= 2;

                double *new_temperature =
                    realloc(temperature,
                            capacity * sizeof(double));

                double *new_humidity =
                    realloc(humidity,
                            capacity * sizeof(double));

                double *new_pressure =
                    realloc(pressure,
                            capacity * sizeof(double));

                double *new_rainfall =
                    realloc(rainfall,
                            capacity * sizeof(double));

                double *new_windspeed =
                    realloc(windspeed,
                            capacity * sizeof(double));


                if (new_temperature == NULL ||
                    new_humidity == NULL ||
                    new_pressure == NULL ||
                    new_rainfall == NULL ||
                    new_windspeed == NULL)
                {
                    printf("Memory reallocation failed.\n");

                    free(new_temperature);
                    free(new_humidity);
                    free(new_pressure);
                    free(new_rainfall);
                    free(new_windspeed);

                    free(temperature);
                    free(humidity);
                    free(pressure);
                    free(rainfall);
                    free(windspeed);

                    fclose(file);

                    return 1;
                }

                temperature = new_temperature;
                humidity = new_humidity;
                pressure = new_pressure;
                rainfall = new_rainfall;
                windspeed = new_windspeed;
            }


            temperature[count] = temp_value;
            humidity[count] = humidity_value;
            pressure[count] = pressure_value;
            rainfall[count] = rainfall_value;
            windspeed[count] = windspeed_value;

            count++;
        }
    }


    fclose(file);


    if (count == 0)
    {
        printf("No valid weather records found.\n");

        free(temperature);
        free(humidity);
        free(pressure);
        free(rainfall);
        free(windspeed);

        return 1;
    }


    /*
       Allocate one statistics structure per thread.

       This avoids race conditions because each thread
       writes only to its own WeatherStats structure.
    */

    WeatherStats *thread_stats =
        malloc(num_threads * sizeof(WeatherStats));

    if (thread_stats == NULL)
    {
        printf("Could not allocate thread statistics.\n");

        free(temperature);
        free(humidity);
        free(pressure);
        free(rainfall);
        free(windspeed);

        return 1;
    }


    for (int i = 0; i < num_threads; i++)
    {
        initialize_stats(&thread_stats[i]);
    }


    /*
       Start timing the computational portion.

       File reading is kept outside this timer so that
       sequential and parallel computational timings
       can be compared consistently.
    */

    double start_time = omp_get_wtime();


    #pragma omp parallel num_threads(num_threads)
    {
        int thread_id = omp_get_thread_num();

        WeatherStats *local_stats =
            &thread_stats[thread_id];


        /*
           Each thread receives a portion of the weather
           observations.

           schedule(static) divides the iterations evenly.
        */

        #pragma omp for schedule(static)
        for (long long i = 0; i < count; i++)
        {
            process_record(
                local_stats,
                temperature[i],
                humidity[i],
                pressure[i],
                rainfall[i],
                windspeed[i]
            );
        }
    }


    /*
       Combine results produced by individual threads.
       This part is sequential but very small compared
       with processing the complete dataset.
    */

    WeatherStats final_stats;
    initialize_stats(&final_stats);


    for (int i = 0; i < num_threads; i++)
    {
        combine_stats(
            &final_stats,
            &thread_stats[i]
        );
    }


    double end_time = omp_get_wtime();

    double execution_time =
        end_time - start_time;


    print_results(
        &final_stats,
        execution_time,
        num_threads
    );


    free(temperature);
    free(humidity);
    free(pressure);
    free(rainfall);
    free(windspeed);
    free(thread_stats);


    return 0;
}