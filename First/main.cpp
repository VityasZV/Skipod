#include <iostream>
#include <vector>
#include <mpi.h>
#include <random>
#include <boost/filesystem.hpp>
#include <fstream>




int main (int argc, char* argv[])
{
    int rank = 0, size = 0, N = 300;
    int cuorum_read = 8, cuorum_write = 5, all_voices = 11;
    int Ts=100, Tb=1;
    //all amount of processes is 12, 0 for main process, 11 - file servers
    MPI_Init (&argc, &argv);      /* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);        /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    MPI_Status status;
    if (rank == 0) {
        //this process attempts to do 3 write operations and 10 read operations
        int read = 0, write=1; 
        std::vector<int> file_version_and_process={0, rank};
        std::cout << "start  " << std::endl;
        for (int i = 0; i < 3; ++i) {
            std::vector<std::vector<int>> file_versions_and_proceses;
            file_versions_and_proceses.resize(cuorum_write);
            std::cout << "before requests" << std::endl; 
            for (int j = 1; j <= cuorum_write; ++j) {
                MPI_Send(&write, 1, MPI_INT, j, 0, MPI_COMM_WORLD); //sending write request to processes 
            }
            std::cout << "after send" << std::endl; 
            for (int j = 1; j <= cuorum_write; ++j) {
                MPI_Recv(&file_version_and_process[j-1], 2, MPI_INT, j, 0, MPI_COMM_WORLD, &status); //getting file_versions and ids of processes
                std::cout << "received " << file_version_and_process[0] << " " << file_version_and_process[1] << std::endl;
                file_versions_and_proceses[j-1] = file_version_and_process;
                std::cout << "received " << file_versions_and_proceses[j-1][0] << " " << file_versions_and_proceses[j-1][1] << std::endl;
            }
            std::cout << "write Request: " << std::endl;
            for (int j = 0 ; j < file_versions_and_proceses.size(); ++j) {
                std::cout << file_versions_and_proceses[j][0] << " " << file_versions_and_proceses[j][1] << std::endl;
            }
        }
    }
    else {
        int request;
        std::vector<int> file_version_and_process={0, rank};
        std::string server_dir = "test" + std::to_string(rank);
        boost::filesystem::create_directory("build/" + server_dir); //creating "server directory"
        boost::filesystem::path full_path(boost::filesystem::current_path()/"build");
        boost::filesystem::copy_file(full_path/"test.txt", full_path/server_dir/("test"+std::to_string(file_version_and_process[0])+".txt"), boost::filesystem::copy_option::overwrite_if_exists);
        // std::fstream file((full_path/server_dir/("test"+std::to_string(0)+".txt")).string());
        // char x;
        // file.read(&x, 1);
        // file.close();
        // std::cout << "process" << std::to_string(rank) << " read " << x << std::endl;
        //Recieve request from master process
        std::cout << "start rank " << rank<< std::endl;
        if (rank >= 1 && rank <= 5) {
            for (int i = 0; i < 3; ++i) {
                MPI_Recv(&request, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
                if (request == 1) {
                    //write request
                    std::cout << "Version and rank = " << file_version_and_process[0] << " " << file_version_and_process[1];
                    MPI_Send(&file_version_and_process, 2, MPI_INT, 0, 0, MPI_COMM_WORLD); 
                    std::cout << "sended from " << rank; 
                }
            }
        } 
    }
    /* get number of processes */
    MPI_Finalize();
    return 0;
}