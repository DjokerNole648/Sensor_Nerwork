#include <stdlib.h>
#include <stdio.h>
#include "sbuffer.h"


int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    pthread_mutex_init(&((*buffer)->head_mutex), NULL);
    pthread_mutex_init(&((*buffer)->tail_mutex), NULL);
    pthread_cond_init(&((*buffer)->read), NULL);
    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    sbuffer_node_t *dummy;
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }
    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }
    pthread_mutex_destroy(&((*buffer)->head_mutex));
    pthread_mutex_destroy(&((*buffer)->tail_mutex));
    pthread_cond_destroy(&((*buffer)->read));
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data, bool isData) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    if (buffer->head == NULL) return SBUFFER_NO_DATA;
    if((buffer->head->data.data_flag == false && isData) || (buffer->head->data.storage_flag == false && !isData)){
        *data = buffer->head->data;
    }
    else{
        return -1;
    }
    if(isData) buffer->head->data.data_flag = true;
    else buffer->head->data.storage_flag = true;
    pthread_mutex_lock(&(buffer)->head_mutex);
    if(buffer->head->data.data_flag && buffer->head->data.storage_flag){
        dummy = buffer->head;
        while (buffer->head == NULL) pthread_cond_wait(&((buffer)->read), &(buffer)->head_mutex);
        if (buffer->head == buffer->tail) // buffer has only one node
        {
            buffer->head = buffer->tail = NULL;
        } else  // buffer has many nodes empty
        {
            buffer->head = buffer->head->next;
        }
        free(dummy);
    }
    pthread_mutex_unlock(&((buffer)->head_mutex));
    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) return SBUFFER_FAILURE;
    dummy->data = *data;
    dummy->next = NULL;
    pthread_mutex_lock(&(buffer)->tail_mutex);
    if (buffer->tail == NULL) // buffer empty (buffer->head should also be NULL)
    {
        buffer->head = buffer->tail = dummy;
    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }
    //unlock all the waiting threads
    pthread_cond_broadcast(&((buffer)->read));
    pthread_mutex_unlock(&(buffer->tail_mutex));
    return SBUFFER_SUCCESS;
}
