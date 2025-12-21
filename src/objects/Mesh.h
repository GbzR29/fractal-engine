#include "Cube.h"

enum faces {
    top, bottom, up, left, right
};

int chunk [16][16][16];

class Mesh{
  public:
    void generateChunk(){

        int x_size = sizeof(chunk) / sizeof(chunk[0]);
        int y_size = sizeof(chunk) / sizeof(chunk[0][0]);
        int z_size = sizeof(chunk) / sizeof(chunk[0][0][0]);

        for (int x = 0; x < x_size; x++)
        {
            for (int y = 0; y < y_size; y++)
            {
                for (int z = 0; z < z_size; z++)
                {
                    
                }
                
            }
            
        }
        

    }
};