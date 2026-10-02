#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <vector>

struct ProcessInfo {
    std::string name;
    pid_t pid;
    bool active;
};

static void childWorker(const std::string &name, int signalPipe) {
    char buffer[64];
    memset(buffer, 0, sizeof(buffer));

    std::cout << "[child] " << name << " created with PID " << getpid() << std::endl;

    ssize_t bytesRead = read(signalPipe, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::cout << "[child] " << name << " received start signal: " << buffer << std::endl;
    }

    sleep(1 + (rand() % 3));
    std::cout << "[child] " << name << " completed execution." << std::endl;
    _exit(0);
}

int main() {
    std::cout << "====================================" << std::endl;
    std::cout << "Process Management in C++" << std::endl;
    std::cout << "====================================" << std::endl;

    std::vector<ProcessInfo> processes;
    std::vector<int> pipeWriters;

    for (int i = 1; i <= 4; ++i) {
        int fd[2];
        if (pipe(fd) == -1) {
            std::cerr << "Pipe creation failed." << std::endl;
            return 1;
        }

        pid_t childPid = fork();
        if (childPid == 0) {
            close(fd[1]);
            childWorker("P" + std::to_string(i), fd[0]);
        }

        close(fd[0]);
        processes.push_back({"P" + std::to_string(i), childPid, true});
        pipeWriters.push_back(fd[1]);
    }

    std::cout << "[parent] Creating and starting process set..." << std::endl;
    for (int writer : pipeWriters) {
        write(writer, "GO", 3);
        close(writer);
    }

    sleep(2);

    std::cout << "[parent] Terminating one process..." << std::endl;
    pid_t terminatedPid = processes[2].pid;
    kill(terminatedPid, SIGTERM);

    int status = 0;
    waitpid(terminatedPid, &status, 0);
    processes[2].active = false;
    std::cout << "[parent] Process P3 terminated." << std::endl;

    std::cout << "[parent] Scheduling remaining processes using round-robin style stop/resume..." << std::endl;
    for (const auto &p : processes) {
        if (!p.active) {
            continue;
        }

        std::cout << "[parent] Pausing process " << p.name << " (PID " << p.pid << ")" << std::endl;
        kill(p.pid, SIGSTOP);
        usleep(300000);
        std::cout << "[parent] Resuming process " << p.name << std::endl;
        kill(p.pid, SIGCONT);
    }

    std::cout << "[parent] Waiting for all running processes to finish..." << std::endl;
    for (auto &p : processes) {
        if (p.active) {
            waitpid(p.pid, &status, 0);
            p.active = false;
        }
    }

    std::cout << "\nProcess management demonstration complete." << std::endl;
    return 0;
}
