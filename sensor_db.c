#include "sensor_db.h"
FILE* fp_data;
char storage_log_event[MAX_LENGTH]; 

void storage_manager(sbuffer_t* sbuffer, int pipefd){
    fp_data = fopen("data.csv", "w");
    sprintf(storage_log_event, "A new data.csv file has been created.\n");
    write_log_event(storage_log_event, pipefd);
    sensor_data_t* sensor_data;
    sensor_data = malloc(sizeof(sensor_data_t));
    while(1){
        sleep(0.5);
        int result = sbuffer_remove(sbuffer, sensor_data, false);
        if(result == SBUFFER_NO_DATA) continue;
        else if(result == -1) continue;
        if(sensor_data->id == 0){
            printf("storagemgr is shutting down.\n");
            break;
        } 
        fprintf(fp_data, "%u %ld %f\n", sensor_data->id, sensor_data->ts, sensor_data->value);
        fflush(fp_data);
        sprintf(storage_log_event, "Data insertion from sensor %" PRIu16 " succeeded.\n", sensor_data->id);
        write_log_event(storage_log_event, pipefd);
    }
    free(sensor_data);
    fclose(fp_data);
    sprintf(storage_log_event, "The data.csv file has been closed.\n");
    write_log_event(storage_log_event, pipefd);
}
