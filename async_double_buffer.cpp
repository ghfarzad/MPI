#include <mpi.h>
#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include <cstdlib>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const char* env_vars[] = {
        "MSMPI_SHM_EAGER_LIMIT",
        "MSMPI_ND_EAGER_LIMIT",
        "MSMPI_SOCKET_EAGER_LIMIT"
    };
    for (const char* var : env_vars) {
        const char* val = std::getenv(var);
        std::cout << "Rank " << rank << " " << var << "="
                  << (val ? val : "(unset)") << std::endl;
    }

    if (size < 2) {
        if (rank == 0) {
            std::cerr << "This program requires at least 2 MPI ranks" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    const int sender = 0;
    const int receiver = 1;
    const std::size_t BUF_SIZE = 1024 * 1024; // 1MB
    const int NUM_ITERS = 10;

    std::vector<char> buffer[2];
    buffer[0].resize(BUF_SIZE);
    buffer[1].resize(BUF_SIZE);

    MPI_Request req[2] = {MPI_REQUEST_NULL, MPI_REQUEST_NULL};

    if (rank == sender) {
        for (int i = 0; i < NUM_ITERS; ++i) {
            int idx = i % 2;
            // Wait for previous send on this buffer to complete
            if (req[idx] != MPI_REQUEST_NULL) {
                MPI_Wait(&req[idx], MPI_STATUS_IGNORE);
            }
            // Fill buffer with data
            std::fill(buffer[idx].begin(), buffer[idx].end(), static_cast<char>('A' + (i % 26)));
            // Asynchronous send
            MPI_Isend(buffer[idx].data(), BUF_SIZE, MPI_CHAR, receiver, 0, MPI_COMM_WORLD, &req[idx]);
            // Simulate work by sleeping 200ms
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        // Ensure all sends complete
        MPI_Waitall(2, req, MPI_STATUSES_IGNORE);
    } else if (rank == receiver) {
        for (int i = 0; i < NUM_ITERS; ++i) {
            int idx = i % 2;
            MPI_Irecv(buffer[idx].data(), BUF_SIZE, MPI_CHAR, sender, 0, MPI_COMM_WORLD, &req[idx]);
            // Wait for current receive to finish
            MPI_Wait(&req[idx], MPI_STATUS_IGNORE);
            // Process data (here we just print the first byte)
            std::cout << "Received iteration " << i << ", first byte: " << buffer[idx][0] << std::endl;
        }
    }

    MPI_Finalize();
    return 0;
}

