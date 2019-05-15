#include<stdio.h>
#include<mpi.h>
#include<math.h>
#include<unistd.h>

int main(int args, char **argc){
    int rank, size;
    int dest, tag = 1, src, job_size = 2;
    int job[3];
    MPI_Status status;

    job[2] = 1; // job status -> job exist

    int dataSizeX = 10, dataSizeY = 10;

    // Init Parallel
    MPI_Init(&args, &argc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Job Distribution
    if (rank==0){ //master
        int num_job = 0;
        double tpart = ceil((double)dataSizeY/job_size); // total partition
        
        // Job Init
        job[0] = 0; // start
        job[1] = (job[0]+job_size)*dataSizeX-1; //end

        // sending first job
        for (dest = 1; dest < size && dest <=tpart; dest++){
            num_job++;
            if (dest == tpart) job[1] = (dataSizeX*dataSizeY)-1;
            MPI_Send(&job, 3, MPI_INT, dest, tag, MPI_COMM_WORLD);
            // printf("Master    : Send job from %d to %d to proc %d\n", job[0], job[1], dest);

            // update job
            job[0] += job_size*dataSizeX;
            job[1] += job_size*dataSizeX;
        }

        // Sending job by request
        for (int part = size-1; part<tpart; part++){
            if (part == tpart-1) job[1] = (dataSizeX*dataSizeY)-1;
            MPI_Recv(&dest, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
            // printf("\nMaster    : Receive job request from Proc %d\n", dest);
            
            MPI_Send(&job, 3, MPI_INT, dest, tag, MPI_COMM_WORLD);
            // printf("Master    : Send job from %d to %d to proc %d\n", job[0], job[1], dest);
            
            // update job
            job[0] += job_size*dataSizeX;
            job[1] += job_size*dataSizeX;

            num_job++;
        }

        //Send to slaves that there's no job left
        job[2] = 0; 
        for (dest=1; dest<size; dest++){
            MPI_Send(&job, 3, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }

        printf("Master    : Number of job %d\n", num_job);
        
    } else { // slaves
        src = 0;
        dest = 0;
        // Receiving first job
        MPI_Recv(&job, 3, MPI_INT, src, tag, MPI_COMM_WORLD, &status);
        
        while (job[2] ==1 ){    
            printf("Slave (%d) : Do job from %d to %d (%d)\n", rank, job[0], job[1], job[2]);   
            // sleep(3); // doing job

            // Request Job
            MPI_Send(&rank, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            // printf("Slave (%d) : Request a job \n", rank);

            // Receiving job by request
            MPI_Recv(&job, 3, MPI_INT, src, tag, MPI_COMM_WORLD, &status);
        }        
        // printf("Slave (%d) : There's no job left \n", rank);
    }

    MPI_Finalize();

    return 0;
}

// compile : mpicc tes.c -lm
// run     : mpiexec -n 4 ./a.out