# ProgettoSO20-21
Phase 1 of the operating systems project for the year 2020/2021.
The goal of this phase is the design of fundamental data structures for the management of processes; specifically a Process Control Block (PCB) list and tree. 
Semaphores are implemented. Each semaphore contains a list of blocked PCBs.

# Implementation details regarding PCBs
pcbFree_h and all process lists are doubly linked and circular.
For each pcb, p_child points to the first node of a doubly linked, NUll-terminated list. New siblings are tail-inserted. Insertion is necessarily O(n) as we only have a head-pointer


# Implementation details regarding ASLs
The only important implementation note worth commenting on concerns the management of the list of active semaphores (semd_h) and, in particular, initASL() and findSemInActiveList().
Two dummy nodes act as sentinels for the beginning and the end of semd_h for easier boundary checking.
Semd_h is kept ordered in ascending order with respect to semAdd, starting from 0 (lowerLimit) up to MAXINT (upperLimit).

# Compiling
Nothing of note.
