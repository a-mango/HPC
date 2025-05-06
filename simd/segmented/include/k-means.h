#include <stdint.h>

#include "image.h"

typedef struct {
    int count;
    int sum_r;
    int sum_g;
    int sum_b;
} ClusterData;

void kmeans_pp(struct img_t *image, int num_clusters, uint8_t *centers);
void kmeans(struct img_t *image, int num_clusters);
