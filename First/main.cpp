#include <iostream>
#include <vector>
#include <mpi.h>
#include <random>
#include <boost/filesystem.hpp>
#include <fstream>
#include <unordered_set>




int main (int argc, char* argv[])
{
    int rank = 0, size = 0, N = 300;
    int cuorum_read = 3, cuorum_write = 10, all_voices = 11;
    int Ts=100, Tb=1, time = 0;
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
                time += Ts;
            }
            std::cout << "after send " << i <<  std::endl; 
            for (int j = 1; j <= cuorum_write; ++j) {
                MPI_Recv(file_version_and_process.data(), 2, MPI_INT, j, 0, MPI_COMM_WORLD, &status); //getting file_versions and ids of processes
                time+=Ts;
                file_versions_and_proceses[j-1] = file_version_and_process;
                std::cout << "received " << file_versions_and_proceses[j-1][0] << " " << file_versions_and_proceses[j-1][1] << std::endl;
            }
            std::cout << "write Request: " << std::endl;
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
                s+=" \n";
                return s;
            }();
            std::cout << "START p=" << process_id  << " " << new_version << std::endl;
            boost::filesystem::path full_path(boost::filesystem::current_path()/"build");
            std::ofstream file((full_path/("test"+std::to_string(process_id))/("test"+std::to_string(new_version-1)+".txt")).string(), std::ios::app);
            file << text;
            time += Tb*300;
            file.close();
            std::cout << "COPY AND REMOVE = " << (full_path/("test"+std::to_string(process_id))/("test"+std::to_string(new_version-1)+".txt")) << " " << (full_path/("test"+std::to_string(process_id))/("test"+std::to_string(new_version)+".txt")) << " " << full_path/("test"+std::to_string(process_id))/("test"+std::to_string(new_version-1)+".txt");
            boost::filesystem::copy_file((full_path/("test"+std::to_string(process_id))/("test"+std::to_string(new_version-1)+".txt")), (full_path/("test"+std::to_string(process_id))/("test"+std::to_string(new_version)+".txt")), boost::filesystem::copy_option::overwrite_if_exists);
            boost::filesystem::remove(full_path/("test"+std::to_string(process_id))/("test"+std::to_string(new_version-1)+".txt"));
            int update = 2;
            for (int p = 1; p <= 11; ++p) {
                MPI_Send(&update, 1, MPI_INT, p, 0, MPI_COMM_WORLD); //sending update request
                time+=Ts;
            }
            std::cout << "INITIAL UPDATE P=" << process_id << " V=" << new_version <<" " <<  std::endl;
            for (int p = 1; p <= 11; ++p) {
                if (p != process_id) {
                    std::cout << "UPDATE FILE=" << new_version << " PID=" << process_id << std::endl;
                    boost::filesystem::copy_file(full_path/("test" + std::to_string(process_id))/("test" + std::to_string(new_version) + ".txt"), 
                                                    full_path/("test" + std::to_string(p))/("test" + std::to_string(new_version) + ".txt"), boost::filesystem::copy_option::overwrite_if_exists);
                    boost::filesystem::remove(full_path/("test" + std::to_string(p))/("test" + std::to_string(new_version-1) + ".txt"));
                }
            }
        }
        for (int i = 0; i < 10; ++i) {
            // std::default_random_engine dre(std::time(nullptr) );
            // std::uniform_int_distribution<> uid(1, 11);
            // std::unordered_set<int> uSet;
            std::vector<int> file_version_and_process={0, rank};

            // while (uSet.size() != cuorum_read)
            //     uSet.insert(uid(dre) );
            std::cout << "Start" << std::endl;
            for (int p = 1; p <= cuorum_read; ++p) {
                MPI_Send(&read, 1, MPI_INT, p, 0, MPI_COMM_WORLD); //sending read request
            }
            std::cout << "SENDED ALL READ" << std::endl;
            std::vector<std::vector<int>> file_versions_and_proceses;
            file_versions_and_proceses.resize(cuorum_read);
            for (int j = 1; j <= cuorum_read; ++j) {
                MPI_Recv(file_version_and_process.data(), 2, MPI_INT, j, 0, MPI_COMM_WORLD, &status); //getting file_versions and ids of processes
                file_versions_and_proceses[j-1] = file_version_and_process;
                std::cout << "received " << file_versions_and_proceses[j-1][0] << " " << file_versions_and_proceses[j-1][1] << std::endl;
            }
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
            int max_version = max_element.first;
            int process_id = max_element.second;
            std::cout <<"READ Request: " << max_version << " " << process_id << std::endl;
            boost::filesystem::path full_path(boost::filesystem::current_path()/"build");
            std::fstream file((full_path/("test" + std::to_string(process_id))/("test"+std::to_string(max_version)+".txt")).string());
            char r;
            for (int k = 1; k < 300; ++k) {
                file.read(&r, 1);
            }
            file.close();
        }
        int stop = 3;
        for (int p = 1; p <= 11; ++p) {
            MPI_Send(&stop, 1, MPI_INT, p, 0, MPI_COMM_WORLD); //sending stop request
        }
        std::cout << "\n Resulted time: " << time << std::endl;
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
            else if (request == 2) {
                //update request
                file_version_and_process[0]+=1;
                std::cout << "UPDATE REQUEST: " << file_version_and_process[0] << " " << file_version_and_process[1] << std::endl;
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