#include <iostream>
#include <vector>
#include <mpi.h>
#include <random>
#include <boost/filesystem.hpp>
#include <fstream>

static std::vector<int> file_array(11, 0); // number of version for each file in each file server

int main (int argc, char* argv[])
{
    int rank = 0, size = 0;
    int cuorum_read = 0, cuorum_write = 0;
    //all amount of processes is 12, 0 for main process, 11 - file servers
    MPI_Init (&argc, &argv);      /* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);        /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    MPI_Status status;

    if (rank == 0) {

    }
    else {
        std::string server_dir = "test" + std::to_string(rank);
        boost::filesystem::create_directory("build/" + server_dir); //creating "server directory"
        boost::filesystem::path full_path(boost::filesystem::current_path()/"build");
        boost::filesystem::copy_file(full_path/"test.txt", full_path/server_dir/("test"+std::to_string(file_array[rank-1])+".txt"), boost::filesystem::copy_option::overwrite_if_exists);
        for (int i = 0; i < rank; ++i) {

        }
    }

    /* get number of processes */
    MPI_Finalize();
    file_array.clear();
    return 0;
}