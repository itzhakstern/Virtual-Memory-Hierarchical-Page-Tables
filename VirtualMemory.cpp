#include "VirtualMemory.h"
#include "PhysicalMemory.h"




/*********************** stetements *************************/



/**
 * the function travel in the tree of pages
 * @param path array with the path in the table page
 * @param depthInPath the current of the depth in the tree
 * @param address the address in the physical table
 * @param virtualAddress the address in the virtual table
 */
void travelsing(int* path, int depthInPath, int* address, uint64_t virtualAddress);
/**
 * the function translate beats
 * @param temp the number to translate
 * @return the offset of the number
 */
uint64_t translateBeats(uint64_t* temp);
/**
 * the function check if the page is empty
 * @param page_num the number of the page
 * @return true if the page is empty, false otherwise
 */
bool isEmptyPage(int page_num);
/**
 * the function calculate the max of tow numbers
 * @param val1 the first number
 * @param val2 the second number
 * @return the max
 */
int max(int val1,int val2);
/**
 * the function calculate the sum of the current path by the formola in the exi
 * @param patharr the path
 * @param page_num the number of the page
 * @return the sum
 */
int calculatePath(const int * patharr,int page_num);
/**
 * the function is a dfs in our mamory tree
 * @param frame_mun the current frame in the dfs
 * @param page_num
 * @param max_frame_index the max index that stile avilable
 * @param parent the parent of the current page
 * @param empty_frame a pointer to struct with data on the page(if we found an empty one)
 * @param victim a pointer to struct with data on the page(if we need to find an victim one)
 * @param offset the offset inside the page
 * @param prevAddress
 * @param d the depth in the dfs
 * @param patharr the path to the current page
 * @param max_path the max page that olready used
 */
void findEmptyTable(word_t* frame_mun,uint64_t page_num, uint64_t &max_frame_index, uint64_t parent, uint64_t* empty_frame,
                    uint64_t* victim, uint64_t offset, uint64_t prevAddress, uint64_t d, uint64_t *patharr, uint64_t &max_path);
/**
 * the function find an inuse frame
 * @param empty_frame a empty struct of empty_frame to fill
 * @param address
 * @param tempAddres
 */
void findUnuseFrame(uint64_t* empty_frame,int * address,uint64_t tempAddres);
/**
 * the function find the path of an unused frame
 * @param path the path of the page
 * @param depthInPath an integer of the depth of the current path
 * @param address
 * @param virtualAddress
 */


/************************* library functions ***************************/



void clearTable(uint64_t frameIndex) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}


void VMinitialize() {
    clearTable(0);
}


int VMread(uint64_t virtualAddress, word_t* value) {
    if(virtualAddress > VIRTUAL_MEMORY_SIZE - 1){
        return 0;
    }
    int pathPages[TABLES_DEPTH];
    uint64_t offset = virtualAddress & ((1 << OFFSET_WIDTH) -1);
    virtualAddress = (virtualAddress >> OFFSET_WIDTH);
    uint64_t tempAddress = virtualAddress;
    for(int i = 1; i <= TABLES_DEPTH; i++)
    {
        pathPages[TABLES_DEPTH-i] = translateBeats(&tempAddress);
    }
    int address = 0;
    for(int i = 0; i < TABLES_DEPTH; i++)
    {
        travelsing(pathPages, i, &address, virtualAddress);
    }
    PMread(address * PAGE_SIZE + offset,value);
    return 1;
}


int VMwrite(uint64_t virtualAddress, word_t value) {
    if(virtualAddress > VIRTUAL_MEMORY_SIZE - 1){
        return 0;
    }
    int pathPages[TABLES_DEPTH];
    uint64_t offset = virtualAddress & ((1 << OFFSET_WIDTH) -1);
    virtualAddress = (virtualAddress >> OFFSET_WIDTH);
    uint64_t tempAddress = virtualAddress;
    for(int i = 1; i <= TABLES_DEPTH; i++)
    {
        pathPages[TABLES_DEPTH-i] = translateBeats(&tempAddress);
    }
    int address = 0;
    for(int i = 0; i < TABLES_DEPTH; i++)
    {
        travelsing(pathPages, i, &address, virtualAddress);
    }
    PMwrite(address * PAGE_SIZE + offset,value);
    return 1;
}


/*************************** helpers ************************/


uint64_t translateBeats(uint64_t* temp){
    uint64_t x = *temp & ((1 << OFFSET_WIDTH) -1);
    *temp = *temp >> OFFSET_WIDTH;
    return x;
}

bool isEmptyPage(int page_num){
    int value;
    for(int i = 0;i<PAGE_SIZE;i++){
        PMread((page_num * PAGE_SIZE) + i, &value);
        if(value){
            return false;
        }
    }
    return true;
}


int max(int val1,int val2){
    return val1 > val2 ? val1 :val2;
}

int calculatePath(const int * patharr,int page_num){
    int odd = 0,even = 0;
    for(int i = 0; i <= TABLES_DEPTH;i++){
        patharr[i] % 2 == 0 ? even += 1 : odd += 1;
    }
    page_num % 2 == 0 ? even += 1 : odd += 1;
    return (odd * WEIGHT_ODD) + (even * WEIGHT_EVEN);
}


void findEmptyTable(int frame_mun,int page_num, int &max_frame_index, int parent, uint64_t* empty_frame, uint64_t* victim, int offset, int prevAddress, int d, int * patharr, int &max_path){
    patharr[d] = frame_mun;
    max_frame_index = max(max_frame_index,frame_mun);
    if(d < TABLES_DEPTH && isEmptyPage(frame_mun) && frame_mun != prevAddress) {
        empty_frame[0] = frame_mun;
        empty_frame[1] = parent;
        empty_frame[2] = offset;
        return;
    }
    if(d == TABLES_DEPTH){
        int curr_max = calculatePath(patharr,page_num);
        if(curr_max > max_path){
            max_path = curr_max;
            victim[0] = frame_mun;
            victim[2] = parent;
            victim[3] = page_num % PAGE_SIZE;
            victim[1] = page_num;
        }
        return;
    }
    int value;
    for(int i = 0;i<PAGE_SIZE;i++){
        PMread(frame_mun * PAGE_SIZE + i, &value);
        if(value){
            findEmptyTable(value,int(page_num * PAGE_SIZE + i), max_frame_index, frame_mun, empty_frame, victim, i, prevAddress, d + 1, patharr, max_path);
        }
    }
}





void findUnuseFrame(int * address, uint64_t tempAddres){
    int max_frame_index = 0,pathArr[TABLES_DEPTH+1],max_path = -1;
    uint64_t victim[5] = {0};
    uint64_t empty_frame[4] = {0};
    findEmptyTable(0, 0, max_frame_index, 0, empty_frame, victim, -1, tempAddres, 0, pathArr, max_path);
    if(empty_frame[0] != 0){
        *address = empty_frame[0];
        PMwrite(empty_frame[1] * PAGE_SIZE + empty_frame[2],0);
    }else if(max_frame_index < NUM_FRAMES-1){
        *address = max_frame_index + 1;
    } else{
        PMevict(victim[0], victim[1]);
        *address = victim[0];
        PMwrite(victim[2] * PAGE_SIZE + victim[3], 0);
    }
}





void travelsing(int* path, int depthInPath, int* address, uint64_t virtualAddress){
    int tempAddress = *address;
    PMread(tempAddress * PAGE_SIZE + path[depthInPath], address);
    if(*address == 0) {
        findUnuseFrame(address, tempAddress);
        if (depthInPath < TABLES_DEPTH - 1) {
            clearTable(*address);
        } else {
            PMrestore(*address, virtualAddress);
        }
        PMwrite(tempAddress * PAGE_SIZE + path[depthInPath], *address);
    }
}