#include <iostream>
#include <cstdlib> // For atoi, rand, srand
#include <ctime>   // For time
#include <mpi.h>

void walker_process();
void controller_process();

int domain_size;
int max_steps;
int world_rank;
int world_size;

int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes and the rank of this process
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (argc != 3)
    {
        if (world_rank == 0)
        {
            std::cerr << "Usage: mpirun -np <p> " << argv[0] << " <domain_size> <max_steps>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    domain_size = atoi(argv[1]);
    max_steps = atoi(argv[2]);

    if (world_rank == 0)
    {
        // Rank 0 is the controller
        controller_process();
    }
    else
    {
        // All other ranks are walkers
        walker_process();
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}

void walker_process()
{
    srand(time(NULL) + world_rank);
    int position = 0;

    for (int step = 1; step <= max_steps; ++step)
    {
        int move = (rand() % 2 == 0) ? -1 : 1;
        position += move;

        if (position < -domain_size || position > domain_size)
        {
            std::cout << "Rank " << world_rank << ": Walker finished in " << step << ".\n";
            int done_signal = step;
            MPI_Send(&done_signal, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            return;
        }
    }

    // Max steps reached
    std::cout << "Rank " << world_rank << ": Walker finished in " << max_steps << ".\n";
    int done_signal = max_steps;
    MPI_Send(&done_signal, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

void controller_process()
{
    int num_walkers = world_size - 1;

    for (int i = 0; i < num_walkers; ++i)
    {
        int received_steps;
        MPI_Status status;
        MPI_Recv(&received_steps, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    }

    std::cout << "Controller: All " << num_walkers << " walkers have finished.\n";
}