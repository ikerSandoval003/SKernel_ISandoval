#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "kernel.h"

/*
SOFTWARE HARIKOPAK (!= HARDWARE HARIKOPAK)
clock: parametro gisa pasatakoa
timer1 (scheduler): baldintzazko mutex bidez komunikatu clock-arekin
timer2 (prozesuak sortzeko): baldintzazko mutex bidez komunikatu clock-arekin

PROZEDURA:
Erlojuak sistemaren denbora kontrolatzen duten zikloak sortzen ditu.
Tenporizadoreek seinale bat (etena) sortzen dute aldiro scheduler-ari eta Prozesu
Sortzaileari deitzeko.
Scheduler-ak seinalea jasotzen duenean, oraingoz, ez du ezer egingo.
Prozesuen sortzaileak, prozesu berrien PCBak sortzen ditu. Oraingoz, PCB-en
eremu bakarra prozesuaren identifikatzailea (pid) izango da.
*/

// usage: kodea.c int main(int argc, char *argv[]) {

// Function prototypes for queue operations

ProcessQueue* queue;
ProcessQueue* queueFIFO;
Machine* makina;
Physical_memory* physical_memory;
Virtual_memory* virtual_memory;
PCB nullProcess = {
    .pid = -1,
    .state = READY,
    .luzera = 0,
    .quantum = 0
};

int kontpid = 0; // kontpid hasieratu
pthread_mutex_t lock; // mutex-a hasieratu
pthread_cond_t cond1, cond2; // kondizioak hasieratu
 int kont_t=0;
void *clock_tread(void *arg);
void *timer_sched(void *arg);
void *timer_proc(void *arg);

int main(int argc, char *argv[]) {
    if (argc != 8) {
        printf("\nEz dira argumentu zuzenak. maizclok maiztimer maizprozedure CPUKOPkop COREKOPkop HARIKOPkop exekuzio_mota \n");
        printf("\n Exekuzio_mota: 0 FCFS 1 Round Robin  2 komplexu(biak konbinatu) \n");
        printf("\n Proiek  \n");
        return 1;
    }
    //atoi aldatu argumentua zenbaki osoan
    int maiztim = atoi(argv[1]); 
    int maizproz = atoi(argv[2]);  
    int maizclock = atoi(argv[3]);   
    printf("queso\n");

     CPUKOP = atoi(argv[4]); 
     COREKOP = atoi(argv[5]);  
     HARIKOP = atoi(argv[6]);  
    EXECMODE = atoi(argv[7]);  
    if(EXECMODE<0 || EXECMODE>2){
        printf("\n Exekuzio mota okerra\n");
        printf("\n Exekuzio_mota: 0 FCFS 1 Round robin  2 komplexu (biak konbinatu) \n");

        return 1;
    }else if(EXECMODE==1 || EXECMODE==2){
             printf("QUANTUM-a sartu nahi duzun \n");
             scanf("%d",&QUANTUM); 
    } 
      HARIKOPTOTAL=CPUKOP*COREKOP*HARIKOP;
   
    
    makina= initialize_machine(CPUKOP,COREKOP,HARIKOP);
    physical_memory = initialize_physical_memory();
    virtual_memory = initialize_virtual_memory();

     

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex hasieraketan arazo bat egon da.\n");
        return 1;
    }

    if (pthread_cond_init(&cond1, NULL) != 0) {
        printf("\n 1. kondizio hasieraketan arazo bat egon da.\n");
        return 1;
    }

    if (pthread_cond_init(&cond2, NULL) != 0) {
        printf("\n 2. kondizio hasieraketan arazo bat egon da.\n");
        return 1;
    }
    
    printf("Sistema martxan jartzen...\n");
    PCB* pcb = create_pcb(kontpid);
    queue =create_queue();
    if(EXECMODE==2){
        queueFIFO=create_queue();
    }
    kontpid++;
    pthread_t p1, p2, p3; // HARIKOPak hasieratu
    pthread_create(&p1, NULL, clock_tread,  (void *)&maizclock); // sortu clock HARIKOPa
    pthread_create(&p2, NULL, timer_sched, (void *)&maiztim); // sortu scheduler HARIKOPa

    //pthread_create(&p2, NULL, timer_sched, (void *)&maiztim); // sortu scheduler HARIKOPa
    pthread_create(&p3, NULL, timer_proc, (void *)&maizproz);
 // sortu prozesu sortzaile haria
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);

    // Garbitu
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond1);
    pthread_cond_destroy(&cond2);
    free(pcb);
        printf("Sistema martxan eeiiii... 3\n");
    free_memory(physical_memory, virtual_memory);
    return 0;
}


//######################################################

//                      TIMERS

//######################################################

void *clock_tread(void *arg) {
    int maiztasuna = *(int *)arg; 
        printf("clock\n");
    int tick=0;
    int minitick=0;
     kont_t = 0;
    while (1) {
             //sleep(1);

        printf("tick %d\n",minitick);
        if(minitick==maiztasuna)
        {

            printf("abisu %d\n",tick);
            minitick=0;
            tick++;          
            pthread_mutex_lock(&lock); // Bloketau mutex.
            while (kont_t < 2) {
                printf("Esperando a abisu %d\n",tick);tick++;
                        // Itxoin 2 timer-ak bukatzera.
                        // kont_t++;

                pthread_cond_wait(&cond1, &lock); // HARIKOPa blokeatu timer-ak kondizioa bete arte. Mutex-a liberatu.
            }

            kont_t = 0; // while-tik atera, kontagailua reiniziatu.

            pthread_cond_broadcast(&cond2); // Kontagailua reiniziatu dela abixatu timer-ei.

            pthread_mutex_unlock(&lock); // mutex-a liberatu.1
            
       }
        minitick++;
        update_processes(makina);
        dispacher();

         
    }
    return NULL;
}

void *timer_sched(void *arg) {
    int maiztasuna = *(int *)arg; // Zenbaki osoa bihurtu
            printf("timer_sched_iniciado\n");
     pthread_mutex_lock(&lock); // clock-ak blokeatutako mutex-ari itxaron.
        int sched_kont=0;
        print_bitmap_status(makina->bitmap);
    while (1) {
               // pthread_mutex_lock(&lock);
       // sleep(1);
        kont_t++; // Kontagailua eguneratu
        sched_kont++;
            if(sched_kont>=maiztasuna){
                printf("Scheduler \n");
                print_bitmap_status(makina->bitmap);
                sched_kont=0;
            }
        pthread_cond_signal(&cond1); // Kondizioa liberatu clock-a jarraitzeko.

        pthread_cond_wait(&cond2, &lock); // Cond2 blokeatu clock-a kontagailua eguneratu arte.
        

    }
     pthread_mutex_unlock(&lock); // mutex-a liberatu.

    return NULL;
}

void *timer_proc(void *arg) {
    int maiztasuna = *(int *)arg; 
            printf("timer_proc\n");
        pthread_mutex_lock(&lock); // clock-ak blokeatutako mutex-a desblokeatu
    int proc_kont=0;
    int processid=0;
    while (1) {
        proc_kont++;
        kont_t++; // Kontagailua eguneratu
            PCB *new_pcb;
        if(proc_kont>=maiztasuna){
            //printf("proceso \n");
            proc_kont=0;
            new_pcb= create_pcb(processid);
            if(EXECMODE==2 && new_pcb->isPriority){
                priority_enqueue(queueFIFO,new_pcb);
            }else if(EXECMODE==2){
                priority_enqueue(queue,new_pcb);
            }else{
                enqueue(queue,new_pcb);
            }
            processid++;

        }
        pthread_cond_signal(&cond1); // Kondizioa liberatu clock-a jarraitzeko.


        pthread_cond_wait(&cond2, &lock); // Cond2 blokeatu clock-a kontagailua eguneratu arte.+


    }
    // pthread_mutex_unlock(&lock); // mutex-a liberatu.

    return NULL;
}

//######################################################

//                      STRUCTURE

//######################################################
Machine* initialize_machine(int num_cpus, int num_cores_per_cup, int num_haris_per_core) {
    // Machine egitura hasieratu
    Machine *machine = (Machine *)malloc(sizeof(Machine));
    if (!machine) {
        perror("Errorea: Machine memoria esleitzeko");
        return NULL;
    }

    machine->num_cpus = num_cpus;
    machine->cpus = (CPU *)malloc(num_cpus * sizeof(CPU));
    int HARIKOPKOPTOTAL = num_cpus * num_cores_per_cup * num_haris_per_core;
    machine->bitmap = bitmap_create(HARIKOPKOPTOTAL);
    if (!machine->cpus) {
        perror("Errorea: CPUKOPs memoria esleitzeko");
        free(machine);
        return NULL;
    }

    // CPU-ak hasieratu
    for (int i = 0; i < num_cpus; i++) {
        machine->cpus[i].num_cores = num_cores_per_cup;
        machine->cpus[i].cores = (Core *)malloc(num_cores_per_cup * sizeof(Core));
        if (!machine->cpus[i].cores) {
            perror("Errorea: COREKOPs memoria esleitzeko");
            free_machine(machine);
            return NULL;
        }

        // Core-ak hasieratu
        for (int j = 0; j < num_cores_per_cup; j++) {
            machine->cpus[i].cores[j].num_haris = num_haris_per_core;
            machine->cpus[i].cores[j].haris = (Hari *)malloc(num_haris_per_core * sizeof(Hari));
            if (!machine->cpus[i].cores[j].haris) {
                perror("Errorea: HARIKOPs memoria esleitzeko");
                free_machine(machine);
                return NULL;
            }

            // Hari bakoitza hasieratu 
            for (int k = 0; k < num_haris_per_core; k++) {
                machine->cpus[i].cores[j].haris[k].pcb.pid = -1; 
            }
        }
    }

    printf("Machine ongi hasieratu da: CPUKOPs=%d, COREKOPs per CPUKOP=%d, HARIKOPs per COREKOP=%d\n",
           num_cpus, num_cores_per_cup, num_haris_per_core);
    return machine;
}

void free_machine(Machine *machine) {
    if (!machine) return;

    // Cores eta Hari bakoitzaren memoria askatu
    for (int i = 0; i < machine->num_cpus; i++) {
        for (int j = 0; j < machine->cpus[i].num_cores; j++) {
            free(machine->cpus[i].cores[j].haris);
        }
        free(machine->cpus[i].cores);
    }

    // CPUs eta Machine askatu
    free(machine->cpus);
    free(machine);

    printf("Machine ongi askatuta\n");
}

// PCB sortu
PCB* create_pcb(int pid) {
    printf("PCB: %d prozesua sortzen \n", pid);
    PCB* new_pcb = (PCB*)malloc(sizeof(PCB));
    if (new_pcb == NULL) {
        perror("Errorea: PCB sortu ezin izan da");
        return NULL;
    }

    // Eremuak ongi hasieratu
    new_pcb->pid = pid;
    new_pcb->state = READY;
    new_pcb->luzera = (rand() % 10) + 1;  // 1-10 bitarteko luzera
    new_pcb->quantum = (EXECMODE == 1 || EXECMODE == 2) ? QUANTUM : 0;
    new_pcb->rr = 0;
    new_pcb->priority = (rand() % 5) + 1;
    new_pcb->isPriority = rand() % 2;
    return new_pcb;
}

//######################################

//          QUEUES 

//######################################

ProcessQueue* create_queue() {
    ProcessQueue *queue = (ProcessQueue *)malloc(sizeof(ProcessQueue));
    if (!queue) {
        perror("Errorea: Ilara sortzerakoan");
        return NULL;
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    pthread_mutex_init(&queue->lock, NULL); // Ilara mutex hasieratu
    return queue;
}

void priority_enqueue(ProcessQueue *queue, PCB *pcb) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node) {
        perror("Errorea: Nodoa sortzeko");
        return;
    }
    new_node->pcb = pcb;
    new_node->next = NULL;

    pthread_mutex_lock(&queue->lock);

    // Ilara hutsa da edo prozesu berriak prioritate handiaena du
    if (is_empty(queue) || pcb->priority > queue->head->pcb->priority) {
        new_node->next = queue->head;
        queue->head = new_node;
        if (queue->tail == NULL) {
            queue->tail = new_node;
        }
    } else {
        // Ordenan sartu
        Node *current = queue->head;
        Node *prev = NULL;
        
        while (current != NULL && current->pcb->priority >= pcb->priority) {
            prev = current;
            current = current->next;
        }
        
        new_node->next = current;
        prev->next = new_node;
        if (current == NULL) {
            queue->tail = new_node;
        }
    }
    
    queue->count++;
    pthread_mutex_unlock(&queue->lock);
}

void enqueue(ProcessQueue *queue, PCB *pcb) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node) {
        perror("Errorea: Nodoa sortzeko");
        return;
    }
    new_node->pcb = pcb;
    new_node->next = NULL;

    pthread_mutex_lock(&queue->lock);
    if (is_empty(queue)) {
        // Ilara hutsa bada, hasiera ezarri
        queue->head = new_node; 
    } else {
        //hurrengo nodo
        queue->tail->next = new_node; 
    }
    
    //tail eguneratu eta kopurua handitu
    queue->tail = new_node; 
    queue->count++; 
    pthread_mutex_unlock(&queue->lock);
}

PCB* dequeue(ProcessQueue *queue) {
    pthread_mutex_lock(&queue->lock);
    if (is_empty(queue)) {
        pthread_mutex_unlock(&queue->lock);
        return NULL; 
    }

    Node *temp = queue->head; // Hasierako nodoa lortu
    PCB *pcb = temp->pcb; // PCB-a hasieratik
    queue->head = queue->head->next; // Hasiera nodoa hurrengo nodoarekin aldatu
    
    // Ilara hutsa bada, tail-a NULL jarri
    if (queue->head == NULL) {
        queue->tail = NULL; 
    }
     // Aurreko nodoa askatu eta kontaketa murriztu
    free(temp);
    queue->count--; 
    pthread_mutex_unlock(&queue->lock);
    return pcb; 
}

// Ilara hutsik dagoen egiaztatu
int is_empty(ProcessQueue *queue) {
    return queue->count == 0;
}

//######################################

//          DISPATCHER (Banatzailea)

//######################################

void dispacher() {

    int hutsik = 0;
    // Prozesuaren hasierako sarrera
    //printf("Dispatcher \n");
    if(EXECMODE == 2){
        if(!is_empty(queueFIFO)){
            while(!hutsik && bitmap_bit_libre(makina->bitmap)){
                assign_processes_to_haris(makina, queueFIFO);
                hutsik = is_empty(queueFIFO);
            }
           printf("Prioritatekoak esleituak eta ilara hutsi dago \n ");;
        }
    }
    if(!is_empty(queue)){
        while(!hutsik && bitmap_bit_libre(makina->bitmap)){
            assign_processes_to_haris(makina, queue);
            hutsik = is_empty(queue);
        }
        printf("Prioiratezkoak ez diren prozezuak esleituta\n");
    }
    printf("Prozesuak esleituak\n");
}

// Prozesuak hari batzuetara esleitzea
void assign_processes_to_haris(Machine *machine, ProcessQueue *queue) {
    if (!machine || !queue || !machine->bitmap) {
        printf("Errorea: Machine, ilara edo bitmap ez daude hasieratuta.\n");
        return;
    }

    while (!is_empty(queue)) {
        PCB *process = dequeue(queue);
        if (!process) break;

        int assigned = 0;
        
        // Prozesua balioztatu esleitu baino lehen
        if (process->pid < 0) {
            sleep(10);
            printf("PID baliogabea detektatu da: %d\n", process->pid);
            continue;
        }

        for (size_t i = 0; i < machine->bitmap->size && !assigned; i++) {
            if (!bitmap_dispo(machine->bitmap, i)) {
                int cpu_index = i / (machine->cpus[0].num_cores * machine->cpus[0].cores[0].num_haris);
                int core_index = (i / machine->cpus[0].cores[0].num_haris) % machine->cpus[0].num_cores;
                int hari_index = i % machine->cpus[0].cores[0].num_haris;

                Hari *current_hari = &machine->cpus[cpu_index].cores[core_index].haris[hari_index];
                
                current_hari->pcb = *process;
                current_hari->pcb.state = RUNNING;
                bitmap_okupatu(machine->bitmap, i);
                printf("Prozesua %d esleitu da CPU %d, Core %d, Hari %d\n", 
                       process->pid, cpu_index, core_index, hari_index);
                
                assigned = 1;
            }
        }

        if (!assigned) {
            printf("Ez dago hari libre prozesu %d-rako.\n", process->pid);
            enqueue(queue, process);
            break;
        }
    }
}
//Prozesuak eguneratu
void update_processes(Machine *machine) {
    if (!machine) return;

    for (int index = 0; index < HARIKOPTOTAL; index++) {
        if (bitmap_dispo(machine->bitmap, index)) {
            int cpu_index = index / (machine->cpus[0].num_cores * machine->cpus[0].cores[0].num_haris);
            int core_index = (index / machine->cpus[0].cores[0].num_haris) % machine->cpus[0].num_cores;
            int hari_index = index % machine->cpus[0].cores[0].num_haris;

            Hari *hari = &machine->cpus[cpu_index].cores[core_index].haris[hari_index];
            
            // Validate PCB before processing
            if (hari->pcb.pid < 0 || hari->pcb.luzera < 0) {
                bitmap_liberatu(machine->bitmap, index);
                hari->pcb = nullProcess;
                continue;
            }

            if (hari->pcb.state == RUNNING) {
                hari->pcb.luzera--;
                  printf("Proesua  %d luzera-aldatuta: %d CPU %d, Core %d, Hari %d.\n",
                           hari->pcb.pid,hari->pcb.luzera, cpu_index, core_index, hari_index);
                if (hari->pcb.luzera <= 0) {
                    printf("Proesua  %d amaitu da:  CPU %d, Core %d, Hari %d.\n",
                           hari->pcb.pid, cpu_index, core_index, hari_index);
                    bitmap_liberatu(machine->bitmap, index);
                    hari->pcb = nullProcess;
                }
                // Prioidadea duten prozesuak exekutatu eta 
                 else if (EXECMODE >= 1 && !hari->pcb.isPriority) {  
                    hari->pcb.quantum--;
                     printf("Proesua  %d Quantum-aldatuta: %d CPU %d, Core %d, Hari %d.\n",
                           hari->pcb.pid,hari->pcb.quantum, cpu_index, core_index, hari_index);
                    if (hari->pcb.quantum <= 0) {
                        printf("Quantum amaitu da  %d  prozesurako:  CPU %d, Core %d, Hari %d.\n",
                               hari->pcb.pid, cpu_index, core_index, hari_index);
                        
                        PCB *pcb_copy = malloc(sizeof(PCB));
                        if (pcb_copy) {
                            *pcb_copy = hari->pcb;
                            pcb_copy->state = READY;
                            pcb_copy->quantum = QUANTUM;
                            
                            // Berriro ilaran sartu
                           if (EXECMODE == 2 && pcb_copy->isPriority) {
                                priority_enqueue(queueFIFO, pcb_copy);
                            } else {
                                enqueue(queue, pcb_copy);
                            }
                            
                            bitmap_liberatu(machine->bitmap, index);
                            hari->pcb = nullProcess;
                        } else {
                            printf("Memoria esleitzeko arazoa\n");
                        }
                    }
                 }
            }
        }
    }
}


//######################################################

//                      BITMAP 

//######################################################



// Bitmapa sortu
Bitmap *bitmap_create(size_t size) {
    Bitmap *bitmap = malloc(sizeof(Bitmap));
    if (!bitmap) {
        perror("Error al asignar memoria para Bitmap");
        return NULL;
    }
    bitmap->size = size;
    size_t byte_size = (size + 7) / 8; 
    bitmap->data = calloc(byte_size, sizeof(unsigned char));
    if (!bitmap->data) {
        free(bitmap);
        perror("Error al asignar memoria para datos del Bitmap");
        return NULL;
    }
    return bitmap;
}

// memoria askatu
void bitmap_free(Bitmap *bitmap) {
    if (bitmap) {
        free(bitmap->data);
        free(bitmap);
    }
}

// bitmapa okupatu
void bitmap_okupatu(Bitmap *bitmap, size_t index) {
    if (index < bitmap->size) {
        bitmap->data[index / 8] |= (1 << (index % 8));
    }
}

// bitmapa askatu
void bitmap_liberatu(Bitmap *bitmap, size_t index) {
    if (index < bitmap->size) {
        bitmap->data[index / 8] &= ~(1 << (index % 8));

    }
}

// Bitmaparen disponibilitatea erakutzi
int bitmap_dispo(Bitmap *bitmap, size_t index) {
    if (index < bitmap->size) {
        return (bitmap->data[index / 8] & (1 << (index % 8))) != 0;
    }
    return -1; 
}

void print_bitmap_status(Bitmap *bitmap) {
    printf("Bitmap egoera: ");
    for (size_t i = 0; i < bitmap->size; i++) {
        printf("%d", bitmap_dispo(bitmap, i) ? 0 : 1);
    }
    printf("\n");
}
// Hari libre dagoen ala ez erakutsi
int bitmap_bit_libre(Bitmap *bitmap) {
    if (!bitmap) return 0;

    for (size_t i = 0; i < bitmap->size; i++) {
        if (bitmap_dispo(bitmap, i) == 0) {
            return 1;
        }
    }

    return 0;
}


















//######################################################

//                      MEMORIA

//######################################################






Physical_memory* initialize_physical_memory() {
    Physical_memory *physical_memory = (Physical_memory *)malloc(sizeof(Physical_memory));
    if (!physical_memory) {
        fprintf(stderr, "Error: Memoria fisikoaren esleipena ezin izan da egin.\n");
        exit(EXIT_FAILURE);
    }

    //  Physical_memory hasieratu
    memset(physical_memory->pid, 0, sizeof(physical_memory->pid));
    for (int i = 0; i < sizeof(physical_memory->frames) / sizeof(Frame); i++) {
        memset(physical_memory->frames[i].data, 0, sizeof(physical_memory->frames[i].data));
    }

    return physical_memory;
}

Virtual_memory* initialize_virtual_memory() {
    Virtual_memory *virtual_memory = (Virtual_memory *)malloc(sizeof(Virtual_memory));
    if (!virtual_memory) {
        fprintf(stderr, "Error: Memoria birtualaren esleipena ezin izan da egin.\n");
        exit(EXIT_FAILURE);
    }

    //  Virtual_memory hasieratu
    memset(virtual_memory->pid, 0, sizeof(virtual_memory->pid));
    for (int i = 0; i < sizeof(virtual_memory->frames) / sizeof(Frame); i++) {
        memset(virtual_memory->frames[i].data, 0, sizeof(virtual_memory->frames[i].data));
    }

    return virtual_memory;
}

// memoria askatu
void free_memory(Physical_memory *physical_memory, Virtual_memory *virtual_memory) {
    if (physical_memory) {
        free(physical_memory);
    }
    if (virtual_memory) {
        free(virtual_memory);
    }
}
//pagina mapeatu
void map_page(PageTable *page_table, int page_number, int frame_number) {
    if (page_number < 0 || page_number >= NUM_PAGES) {
        fprintf(stderr, "Error: Orri zenbakia tartetik kanpo.\n");
        return;
    }

    page_table->orriak[page_number].frame_number = frame_number;
    page_table->orriak[page_number].valid = 1; 
}

int translate_address(PageTable *page_table, int virtual_address) {
    int page_number = virtual_address / PAGE_SIZE;       
    int offset = virtual_address % PAGE_SIZE;            

    if (page_number < 0 || page_number >= NUM_PAGES) {
        fprintf(stderr, "Error: Helbide birtuala tartetik kanpo.\n");
        return -1;
    }

    Orria entry = page_table->orriak[page_number];
    if (!entry.valid) {
        fprintf(stderr, "Error: orri ez baliozkoa.\n");
        return -1;
    }

    int physical_address = (entry.frame_number * PAGE_SIZE) + offset;
    return physical_address;
}
