#ifndef PROCESS_QUEUE_H
#define PROCESS_QUEUE_H

#include <pthread.h>

// ----------------- ENUMERAZIOAK ETA EGITURAK -----------------
#define MAX_NAME_LENGTH 32
int HARIKOPTOTAL;
int EXECMODE,CPUKOP,COREKOP,HARIKOP,QUANTUM;

#define PHYSICAL_MEMORY_SIZE 65536  // 4 hex karaktere: 2^16 bytes
#define PAGE_SIZE 4
#define NUM_PAGES (PHYSICAL_MEMORY_SIZE / PAGE_SIZE)

typedef unsigned char byte;

//byte physical_memory[PHYSICAL_MEMORY_SIZE];

typedef struct {
    int frame_number; // Zenbakia esleitu zaion marko fisikoa
    int valid;        // Baliozkotasun bit-a: 1 baldin eta orria mapetuta badago, 0 ez
} Orria;

typedef struct {
    Orria orriak[NUM_PAGES]; // Orrien taula NUM_PAGES sarrerarekin
} PageTable;

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    FINISHED
} ProcessState;

typedef struct {
    unsigned int TLB;
} MMU;

typedef struct {
    char data[MAX_NAME_LENGTH];
    unsigned int code;
    unsigned int pgb;
} MM;

typedef struct{
    char data[64]; // 64 byte datuak
} Frame;

typedef struct{
    char pid[4];   // pid  0x 0000 0001 
    Frame frames[PHYSICAL_MEMORY_SIZE]; // markoen kopurua 4 digitoko hexadekimala
} Physical_memory;

typedef struct{
    char pid[4];   // pid  0x 0000 0001 
    Frame frames[PHYSICAL_MEMORY_SIZE]; // markoen kopurua 4 digitoko hexadekimala
} Virtual_memory;

typedef struct {
    int pid;                     // Prozesuaren identifikatzailea
    ProcessState state;          // Prozesuaren egoera
    int luzera;
    int priority;                // Prozesuaren lehentasuna
    int quantum;
    int rr;
    int isPriority;
    //MM mm
} PCB;

typedef struct Node {
    PCB *pcb;                    // PCB-rako seinale
    struct Node *next;           // Hurrengo nodoaren seinale
} Node;

typedef struct {
    Node *head;                 // Ilara buruaren seinale
    Node *tail;                 // Ilara bukaeraren seinale
    int count;                  // Ilaran dauden elementuen kopurua
    pthread_mutex_t lock;       // Threadeko segurtasunerako mutex-a
} ProcessQueue;



#endif // PROCESS_QUEUE_H

typedef struct {
    unsigned int PC; // Programaren Kontagailua
    unsigned int IR; // Instrukzioaren Erregistroa
    MMU mmu;
    PCB pcb; // "Hari" bakoitzak PCB bat du lotuta
} Hari;

typedef struct {
    Hari *haris; // "Haris"-en array dinamikoa
    int num_haris; // Harien kopurua
} Core;

typedef struct {
    Core *cores; // "Core"-en array dinamikoa
    int num_cores; // Core kopurua
} CPU;

typedef struct Bitmap {
    unsigned char *data; // Byte arraya, bit-ak gordetzeko
    size_t size;         // Bit guztien kopurua
} Bitmap;

typedef struct {
    CPU *cpus; // "CPU"-en array dinamikoa
    int num_cpus; // CPU kopurua
    Bitmap *bitmap;
} Machine;

/// ----------------- FUNTZIOAK -----------------

// Prozesuen ilara
ProcessQueue* create_queue();
void enqueue(ProcessQueue *queue, PCB *pcb); // Prozesua ilaran sartu
PCB* dequeue(ProcessQueue *queue);//   ilaratik atera
int is_empty(ProcessQueue *queue); // Ilara hutsa den ala ez

// PCB
PCB* create_pcb(int pid);   // PCB sortu
// Machine egitura hasieratzeko funtzioa
Machine* initialize_machine(int num_cpus, int num_cores_per_cpu, int num_haris_per_core);   // Machine egitura hasieratu

// Machine egiturari esleitutako memoria askatzeko funtzioa
void free_machine(Machine *machine);    // Machine egitura askatu
void dispacher();                // Prozesuak harietara esleitu
void assign_processes_to_haris(Machine *machine, ProcessQueue *queue);              // Prozesuak harietara esleitu
void update_processes();// Prozesuak eguneratu
Bitmap *bitmap_create(size_t size);     // Bitmapa sortu
void bitmap_free(Bitmap *bitmap);           // Bitmapa askatu
void bitmap_okupatu(Bitmap *bitmap, size_t index);  // Bitmapa okupatu                               
void bitmap_liberatu(Bitmap *bitmap, size_t index); // Bitmapa askatu
int bitmap_dispo(Bitmap *bitmap, size_t index);   // Bitmapa libre dagoen index horretan

int is_pid_in_queue(ProcessQueue *queue, int pid) {
    if (queue == NULL || queue->head == NULL) return 0;

    Node *current = queue->head;
    while (current != NULL) {
        PCB *current_pcb = current->pcb;
        if (current_pcb->pid == pid) {
            return 1;  // PID berdina topatu bada
        }
        current = current->next;
    }
    return 1;  // PID-a ez da aurkitu
}

Physical_memory* initialize_physical_memory();    // Fisikako memoria hasieratu                                                      
Virtual_memory* initialize_virtual_memory();      // Birtuako memoria hasieratu                                                                   
void free_memory(Physical_memory *physical_memory, Virtual_memory *virtual_memory); // Memoria askatu
