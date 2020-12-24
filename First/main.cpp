#include <iostream>
#include <vector>
#include <mpi.h>
#include <random>
#include <boost/filesystem.hpp>
#include <fstream>




int main (int argc, char* argv[])
{
    int rank = 0, size = 0, N = 300;
    int cuorum_read = 3, cuorum_write = 10, all_voices = 11;
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
                MPI_Recv(file_version_and_process.data(), 2, MPI_INT, j, 0, MPI_COMM_WORLD, &status); //getting file_versions and ids of processes
                std::cout << "received1 " << file_version_and_process[0] << " " << file_version_and_process[1] << std::endl;
                file_versions_and_proceses[j-1] = file_version_and_process;
                std::cout << "received2 " << file_versions_and_proceses[j-1][0] << " " << file_versions_and_proceses[j-1][1] << std::endl;
            }
            std::cout << "write Request: " << std::endl;
            for (int j = 0 ; j < file_versions_and_proceses.size(); ++j) {
                std::cout << file_versions_and_proceses[j][0] << " " << file_versions_and_proceses[j][1] << std::endl;
                auto max_element = [&](){
                    int max_version = 0, process_id = 0;
                    for (auto &v_p : file_versions_and_proceses) {
                        if (v_p[0] >= max_version) {
                            max_version = v_p[0];
                            process_id = v_p[1];
                        }
                    }
                    return std::pair<int, int>{max_version, process_id};
                }();
                int new_version = max_element.first+1;
                int process_id = max_element.second;
                std::string text = [](){
                    auto s = std::string("f");
                    for (int i = 1; i <= 300; ++i) {
                        s = s + "f";
                    }
                    return s;
                }();
                boost::filesystem::path full_path(boost::filesystem::current_path()/"build");
                std::fstream file((full_path/("test"+std::to_string(process_id))/("test"+std::to_string(new_version-1)+".txt")).string());
                file << text;
                file.close();
                boost::filesystem::copy_file((full_path/("test"+std::to_string(process_id))/("test"+std::to_string(new_version-1)+".txt")), (full_path/("test"+std::to_string(process_id))/("test"+std::to_string(new_version)+".txt")), boost::filesystem::copy_option::overwrite_if_exists);
                boost::filesystem::remove(full_path/("test"+std::to_string(process_id))/("test"+std::to_string(new_version-1)+".txt"));
                int update = 2;
                for (int p = 1; p <= 12; ++p) {
                    MPI_Send(&update, 1, MPI_INT, p, 0, MPI_COMM_WORLD); //sending update request
                }
                std::vector<int> v_p;
                v_p.resize(2);
                v_p[0] = new_version;
                v_p[1] = process_id;
                for (int p = 1; p <= 12; ++p) {
                    MPI_Send(&update, 1, MPI_INT, p, 0, MPI_COMM_WORLD); //sending update request
                }
                for (int p = 1; p <= 12; ++p) {
                    MPI_Send(&v_p, 2, MPI_INT, p, 0, MPI_COMM_WORLD); //sending update request
                }
                // char x;
                // file.read(&x, 1);
                // file.close();
                // std::cout << "process" << std::to_string(rank) << " read " << x << std::endl;
                //Recieve request from master process
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
        while (true) {
            MPI_Recv(&request, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            if (request == 1) {
                //write request
                std::cout << "Version and rank = " << file_version_and_process[0] << " " << file_version_and_process[1];
                MPI_Send(file_version_and_process.data(), 2, MPI_INT, 0, 0, MPI_COMM_WORLD); 
                std::cout << "sended from " << rank; 
            }
            else if (request == 0) {
                //read request
                std::cout << "Version and rank = " << file_version_and_process[0] << " " << file_version_and_process[1];
                MPI_Send(file_version_and_process.data(), 2, MPI_INT, 0, 0, MPI_COMM_WORLD); 
            }
            else if (request == 2){
                //update request 
                std::vector<int> file_v_and_pid (2, 0);
                MPI_Recv(file_v_and_pid.data(), 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
                boost::filesystem::copy_file(full_path/("test" + std::to_string(file_v_and_pid[1]))/("test" + std::to_string(file_v_and_pid[0]) + ".txt"), full_path/server_dir/("test"+std::to_string(file_v_and_pid[0])+".txt"), boost::filesystem::copy_option::overwrite_if_exists);
                boost::filesystem::remove(full_path/server_dir/("test"+std::to_string(file_v_and_pid[0])+".txt"));
            }
            else if (request == 3) {
                //stop request
                break;
            }
        }
    }
    /* get number of processes */
    MPI_Finalize();
    return 0;
}