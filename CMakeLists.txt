#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

struct Process {
    std::string name;
    int size;
};

struct MemoryBlock {
    int start;
    int size;
    std::string owner;
};

struct AllocationResult {
    std::string algorithm;
    int totalMemory;
    int usedMemory;
    int fragmentation;
    std::vector<std::string> unallocated;
    std::map<std::string, std::pair<int, int>> allocations;
    double executionTime;
};

static int findFirstFit(const std::vector<MemoryBlock> &blocks, int requiredSize) {
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].size >= requiredSize) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

static int findBestFit(const std::vector<MemoryBlock> &blocks, int requiredSize) {
    int bestIndex = -1;
    int bestSize = INT_MAX;

    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].size >= requiredSize && blocks[i].size < bestSize) {
            bestSize = blocks[i].size;
            bestIndex = static_cast<int>(i);
        }
    }

    return bestIndex;
}

static int findWorstFit(const std::vector<MemoryBlock> &blocks, int requiredSize) {
    int worstIndex = -1;
    int worstSize = -1;

    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].size >= requiredSize && blocks[i].size > worstSize) {
            worstSize = blocks[i].size;
            worstIndex = static_cast<int>(i);
        }
    }

    return worstIndex;
}

static void mergeFreeBlocks(std::vector<MemoryBlock> &blocks) {
    std::sort(blocks.begin(), blocks.end(), [](const MemoryBlock &a, const MemoryBlock &b) {
        return a.start < b.start;
    });

    std::vector<MemoryBlock> merged;
    for (const auto &block : blocks) {
        if (merged.empty()) {
            merged.push_back(block);
            continue;
        }

        MemoryBlock &last = merged.back();
        if (last.owner == "free" && block.owner == "free" && last.start + last.size == block.start) {
            last.size += block.size;
        } else {
            merged.push_back(block);
        }
    }

    blocks.swap(merged);
}

static void deallocateProcess(std::vector<MemoryBlock> &blocks, const std::map<std::string, std::pair<int, int>> &allocations,
                             const std::string &name) {
    auto it = allocations.find(name);
    if (it == allocations.end()) {
        return;
    }

    int start = it->second.first;
    int end = it->second.second;
    blocks.push_back({start, end - start, "free"});
    mergeFreeBlocks(blocks);
}

static AllocationResult simulateAllocation(int totalMemory, const std::vector<Process> &processes,
                                          const std::string &algorithm) {
    std::vector<MemoryBlock> freeBlocks = {{0, totalMemory, "free"}};
    std::map<std::string, std::pair<int, int>> allocations;
    std::vector<std::string> unallocated;
    int usedMemory = 0;

    auto startTime = std::chrono::high_resolution_clock::now();

    int (*selector)(const std::vector<MemoryBlock> &, int) = nullptr;
    if (algorithm == "first-fit") {
        selector = findFirstFit;
    } else if (algorithm == "best-fit") {
        selector = findBestFit;
    } else if (algorithm == "worst-fit") {
        selector = findWorstFit;
    } else {
        throw std::invalid_argument("Unsupported algorithm: " + algorithm);
    }

    for (const auto &process : processes) {
        int index = selector(freeBlocks, process.size);
        if (index == -1) {
            unallocated.push_back(process.name);
            continue;
        }

        MemoryBlock block = freeBlocks[index];
        int startAddress = block.start;
        int endAddress = block.start + process.size;
        allocations[process.name] = {startAddress, endAddress};
        usedMemory += process.size;

        freeBlocks.erase(freeBlocks.begin() + index);
        if (block.size > process.size) {
            freeBlocks.insert(freeBlocks.begin() + index, {endAddress, block.size - process.size, "free"});
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double executionTime = std::chrono::duration<double, std::milli>(endTime - startTime).count() / 1000.0;

    int fragmentation = 0;
    for (const auto &block : freeBlocks) {
        fragmentation += block.size;
    }

    return {algorithm, totalMemory, usedMemory, fragmentation, unallocated, allocations, executionTime};
}

static void printResult(const AllocationResult &result) {
    double utilization = (result.usedMemory * 100.0) / result.totalMemory;
    std::cout << "Algorithm: " << result.algorithm << std::endl;
    std::cout << "  Memory utilization: " << utilization << "%" << std::endl;
    std::cout << "  Used memory: " << result.usedMemory << std::endl;
    std::cout << "  Fragmentation: " << result.fragmentation << std::endl;
    std::cout << "  Execution time: " << result.executionTime << "s" << std::endl;
    std::cout << "  Unallocated: " << (result.unallocated.empty() ? "None" : "") << std::endl;
    for (const auto &name : result.unallocated) {
        std::cout << "    - " << name << std::endl;
    }
    std::cout << "  Allocation map:" << std::endl;
    for (const auto &[name, range] : result.allocations) {
        std::cout << "    " << name << " -> [" << range.first << ", " << range.second << ")" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "====================================" << std::endl;
    std::cout << "Memory Management Simulation in C++" << std::endl;
    std::cout << "====================================" << std::endl;

    std::vector<std::vector<Process>> testCases = {
        { {"P1", 20}, {"P2", 10}, {"P3", 15}, {"P4", 5}, {"P5", 30}, {"P6", 8} },
        { {"A1", 40}, {"A2", 20}, {"A3", 35}, {"A4", 10}, {"A5", 15} },
        { {"B1", 12}, {"B2", 18}, {"B3", 5}, {"B4", 30}, {"B5", 14} }
    };

    for (size_t caseIndex = 0; caseIndex < testCases.size(); ++caseIndex) {
        std::cout << "\n=== Test Case " << (caseIndex + 1) << " ===" << std::endl;
        std::cout << "Total memory: 100" << std::endl;
        std::cout << "Processes: ";
        for (const auto &process : testCases[caseIndex]) {
            std::cout << process.name << "(" << process.size << ") ";
        }
        std::cout << std::endl << std::endl;

        std::vector<std::string> algorithms = {"first-fit", "best-fit", "worst-fit"};
        for (const auto &algorithm : algorithms) {
            AllocationResult result = simulateAllocation(100, testCases[caseIndex], algorithm);
            printResult(result);
        }
    }

    std::cout << "Memory management demonstration complete." << std::endl;
    return 0;
}
