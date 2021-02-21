# ProgettoSO20-21
First part of the operating systems project for the year 2020/2021.
This phase is dedicated to the construction of fundamental data structures for the management of processes. 
Specifically a PCB (Process Control Block) list and a PCB tree. 
In addition, the semd_t type was used to create a list of semaphores for process management, each with a value and a list of blocked PCBs.

# Implementation details for the part relating to the PCBs

# Implementation details for the part relating to the ASLs
The only important implementation note worth commenting on concerns the management of the list of active semaphores (semd_h).
In particular within initASL() which takes care of initializing the structures relating to the ASLs.
Here it is possible to notice that two semaphores are instantiated that act as sentinel for the beginning and the end of semd_h.
This was a design choice made to ensure that the search within the list of active semaphores was as optimized as possible. semd_h is therefore ordered in ascending order with respect to the value of the address of semAdd, by a value of 0 (lowerLimit) and a maximum value given by the specific bitmask (upperLimit).
Thanks to this type of implementation, the findSemInActiveList() function returns the semaphore preceding the one requested, (whether this is within the list of active semaphores, or not) allowing us to easily carry out operations of insertion and removal from the list, without risking to go outside it.

# Compilation
