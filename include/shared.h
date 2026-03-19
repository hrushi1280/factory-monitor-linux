#ifndef SHARED_H
#define SHARED_H

#define SHM_KEY 1234
#define SEM_KEY 5678

struct sensor_data
{
    int temperature;
    int vibration;
    int load;
};

#endif
