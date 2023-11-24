#include <stdio.h>
#include <pthread.h>

struct Process {
  int process_id;
  int burst_time;
  int remaining_time;
  int arrival_time;
  int completion_time;
  int memory_size;  // New attribute for memory management
};

struct MemoryPartition {
  int partition_id;
  int size;
  int allocated_process_id;
};

struct ThreadData {
  struct Process* process;
  int quantum;
  struct MemoryPartition* memory_partitions;
  int num_partitions;
  pthread_mutex_t* memory_mutex;
};

void* processThread(void* arg) {
  struct ThreadData* data = (struct ThreadData*)arg;
  struct Process* process = data->process;
  int quantum = data->quantum;

  // Implement the Round Robin scheduling algorithm for a single process
  while (process->remaining_time > 0) {
    if (process->remaining_time > quantum) {
      process->remaining_time -= quantum;
    } else {
      process->completion_time = process->arrival_time + process->burst_time;
      process->remaining_time = 0;
    }
  }

  // Simulate memory deallocation
  pthread_mutex_lock(data->memory_mutex);
  for (int i = 0; i < data->num_partitions; i++) {
    if (data->memory_partitions[i].allocated_process_id == process->process_id) {
      data->memory_partitions[i].allocated_process_id = -1;
      break;
    }
  }
  pthread_mutex_unlock(data->memory_mutex);

  pthread_exit(NULL);
}

void allocateMemory(struct Process* process, struct MemoryPartition* memory_partitions, int num_partitions, pthread_mutex_t* memory_mutex) {
  // Simulate memory allocation
  pthread_mutex_lock(memory_mutex);
  for (int i = 0; i < num_partitions; i++) {
    if (memory_partitions[i].allocated_process_id == -1 && memory_partitions[i].size >= process->memory_size) {
      memory_partitions[i].allocated_process_id = process->process_id;
      pthread_mutex_unlock(memory_mutex);
      return;
    }
  }
  pthread_mutex_unlock(memory_mutex);
  printf("Memory allocation failed for Process %d\n", process->process_id);
}

void roundRobinMultithreaded(struct Process processes[], int n, int quantum, struct MemoryPartition memory_partitions[], int num_partitions, pthread_mutex_t memory_mutex) {
  pthread_t threads[n];
  struct ThreadData threadData[n];

  // Create threads for each process
  for (int i = 0; i < n; i++) {
    threadData[i].process = &processes[i];
    threadData[i].quantum = quantum;
    threadData[i].memory_partitions = memory_partitions;
    threadData[i].num_partitions = num_partitions;
    threadData[i].memory_mutex = &memory_mutex;

    // Allocate memory for each process
    allocateMemory(&processes[i], memory_partitions, num_partitions, &memory_mutex);

    pthread_create(&threads[i], NULL, processThread, (void*)&threadData[i]);
  }

  // Wait for all threads to finish
  for (int i = 0; i < n; i++) {
    pthread_join(threads[i], NULL);
  }
}

int main() {
  int n, quantum, num_partitions;
  printf("Enter the number of processes: ");
  scanf("%d", &n);

  printf("Enter the number of memory partitions: ");
  scanf("%d", &num_partitions);

  struct Process processes[n];
  struct MemoryPartition memory_partitions[num_partitions];

  // Initialize memory partitions
  for (int i = 0; i < num_partitions; i++) {
    memory_partitions[i].partition_id = i + 1;
    memory_partitions[i].size = 100;  // Set a default size for each partition
    memory_partitions[i].allocated_process_id = -1;
  }

  printf("Enter the burst time, arrival time, and memory size for each process:\n");
  for (int i = 0; i < n; i++) {
    processes[i].process_id = i + 1;
    printf("Burst time for process %d: ", i + 1);
    scanf("%d", &processes[i].burst_time);
    printf("Arrival time for process %d: ", i + 1);
    scanf("%d", &processes[i].arrival_time);
    printf("Memory size for process %d: ", i + 1);
    scanf("%d", &processes[i].memory_size);
    processes[i].remaining_time = processes[i].burst_time;
  }

  printf("Enter the time quantum: ");
  scanf("%d", &quantum);

  printf("\nSimulation Results (Multithreaded Round Robin with Memory Management):\n");

  roundRobinMultithreaded(processes, n, quantum, memory_partitions, num_partitions, (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER);

  printf("\nProcess ID\tArrival Time\tBurst Time\tMemory Size\tCompletion Time\n");
  for (int i = 0; i < n; i++) {
    printf("%d\t\t%d\t\t%d\t\t%d\t\t%d\n", processes[i].process_id, processes[i].arrival_time, processes[i].burst_time, processes[i].memory_size, processes[i].completion_time);
  }

  return 0;
}