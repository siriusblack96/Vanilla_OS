/*
     File        : file_system.C
     Author      : Riccardo Bettati
     Modified    : 2021/11/28
     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#define FREE_BLOCK_IDENTIFIER 'f'
#define USED_BLOCK_IDENTIFIER 'u'
#define INODE_STORING_BLOCK_NO 1
#define FREE_BLOCK_BITMAP_BLOCK_NO 0

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/


Inode::Inode()
{
  fs = NULL;
  inode_free = true;
  file_size= 0;
  id = -1;
}



/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    disk = NULL;
    size = 0;
    free_block_count = SimpleDisk::BLOCK_SIZE/sizeof(unsigned char);
    free_blocks = new unsigned char[free_block_count];
    inode_counter= 0;
    inodes = new Inode[MAX_INODES];
  
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */
    disk->write(FREE_BLOCK_BITMAP_BLOCK_NO,free_blocks);
    unsigned char* tmp = (unsigned char*) inodes;
    disk->write(INODE_STORING_BLOCK_NO,tmp);
    
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");

    /* Here you read the inode list and the free list into memory */
    
    disk = _disk;
    unsigned char* tmp;
    _disk->read(FREE_BLOCK_BITMAP_BLOCK_NO,free_blocks);
    _disk->read(INODE_STORING_BLOCK_NO,tmp);
    inodes = (Inode *) tmp;
    inode_counter= 0;

    for (int i ;i<MAX_INODES ;i++){
      if (!inodes[i].inode_free){
        inode_counter++;
        //break;
      }
    }

    free_block_count = SimpleDisk::BLOCK_SIZE/sizeof(unsigned char) ;
    // free block count??
    return true;

}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
    int count_of_free_blocks = _size/SimpleDisk::BLOCK_SIZE;
    unsigned char* array_for_free_blocks = new unsigned char[count_of_free_blocks];
    
    array_for_free_blocks[0] = USED_BLOCK_IDENTIFIER;
    array_for_free_blocks[1] = USED_BLOCK_IDENTIFIER;
    
    for (int i=2;i<count_of_free_blocks;i++){
      array_for_free_blocks[i]=FREE_BLOCK_IDENTIFIER;
    }

    _disk->write(FREE_BLOCK_BITMAP_BLOCK_NO,array_for_free_blocks);
    Inode* tmp_inodes = new Inode[MAX_INODES];
    unsigned char* tmp = (unsigned char*) tmp_inodes;
    _disk->write(INODE_STORING_BLOCK_NO,tmp);
    return true;

}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    for (int i = 0;i++;i<inode_counter){
      if (inodes[i].id==_file_id){
        return &inodes[i];
      }

    }
    assert(false);
    return NULL;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    
    for (int i = 0;i++;i<inode_counter){
      if (inodes[i].id==_file_id){
        assert(false);
      }

    }
    
    int free_inode_index = -1;
    for (int i = 0;i<MAX_INODES;i++){
      if (inodes[i].inode_free){
        free_inode_index = i;
        break;
      }
    }
    
    int free_block_index = -1;

    for (int i = 0;i<free_block_count;i++){
      if (free_blocks[i]==FREE_BLOCK_IDENTIFIER){
        free_block_index = i;
        break;
      }
    }
	  
	  
    //Console::puti(free_block_index); Console::puts(" block\n");
    //Console::puti(free_inode_index); Console::puts(" inode\n");
    
    assert((free_inode_index!=-1) && (free_block_index!=-1));
    //Inode inode_found = inodes[free_inode_index];
    inodes[free_inode_index].inode_free = false;
    inodes[free_inode_index].fs = this;
    inodes[free_inode_index].block_no = free_block_index;
    inodes[free_inode_index].id = _file_id;
    free_blocks[free_block_index] = USED_BLOCK_IDENTIFIER;
    
    //Console::puti(inodes[0].id); Console::puts("\n");
    //inode_found->InitialiseInode(_file_id,  free_block_index, this );
    
    return true;

    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */

    bool file_found = false;
    int delete_inode_index;


    for (int i = 0;i<MAX_INODES;i++){
      if (inodes[i].id==_file_id){
        delete_inode_index = i;
        file_found = true;
        break;
      }
    }

    assert(file_found);
    

    int block_no = inodes[delete_inode_index].block_no;
    free_blocks[block_no] = FREE_BLOCK_IDENTIFIER;
    inodes[delete_inode_index].inode_free = true;
    inodes[delete_inode_index].file_size = 0;

    return true;


}
