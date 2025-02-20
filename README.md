# File System

## 📌 Project Overview

This project was created as part of a **Operating Systems** course at university during the **3rd semester**. It mainly focuses on the project of a file system, providing a structured overview of its architecture and operations.

implementation only supports the delete operation on a pre-prepared disk image (disk_start_state.bin).
## 📌 Parameters

- **Allocation block size**: 16 × 8B = 128B = 16B
- **Number of allocation blocks**: 2⁷ = 128
- **Bits required to describe a block address**: 8b = 1B
- **Memory occupied**: 2357 B
- **Actual memory size**: 2048 B = 2 KiB
- **Memory utilization efficiency**: 87%

---

## 🏗️ Architecture

### **Control Block** (5B) `[0; 4]`

- **For writing:**
  - Transaction flag (0) - 1B
  - File descriptor index - 1B
  - New size - 2B
  - Start of file (if old size = 0) - 1B
- **For deleting a file:**
  - Transaction flag (1) - 1B
  - File descriptor index - 1B
  - Current block address - 1B
  - Next block address - 1B

### **File Descriptor Table** (11B × 16 = 176B) `[5; 180]`

Each stored file has its descriptor in this table:

- **File name** (max. 8 characters) - 8B
- **First allocation block address** - 1B
- **File size** - 2B

### **FAT Table** (1B × 128 = 128B) `[181; 308]`

The file system uses a **linked list structure** for allocation, with FAT mapping:

- `-2` → Free block
- `-1` → Last block in the chain
- `n` → Address of the next allocation block

### **Disk Space** (16B × 128 = 2048B = 2 KiB) `[309; 2356]`

---

## 🔧 Operations

### **File Operations:**

- **Write to file**
- **Open file**
- **Close file**
- **Create file**
- **Delete file**
- **Read from file**

### **Opening a File**

1. Locate the file descriptor by name.
2. Access the **open file descriptors table**.
3. Find the first available slot (where descriptor ID `< 0`).
4. Set the pointer to `0` and assign the descriptor index.

### **Closing a File**

1. Find the file descriptor in the **open file descriptors table**.
2. Set its ID to `-1`.

### **Creating a File**

1. Find the first available descriptor (`first block address = -2`).
2. Assign the file name and set its size to `0`.
3. Set the first block address to `-1`.

### **Deleting a File**

1. Locate the file descriptor.
2. If the file size is `0`, proceed to step 9; otherwise, continue.
3. Add the descriptor index and the first block address to the control block.
4. Set the **delete flag**.
5. If not the last block, add the next block address to the control block.
6. Set the **FAT table value of the current block to ****`-2`** (mark as free).
7. Assign the next block address to the current block.
8. If FAT values for both current and next blocks ≠ `-2`, repeat from step 6.
9. Set the first block address in the descriptor table to `-2`.
10. Remove the transaction flag.

**Recovery after interruption:** Restart from step 5.

### **Reading from a File**

1. Determine the block number based on file position.
2. Read data into the buffer until **remaining size = 0** or **end of file**.
3. If at the end of an allocation block, update size and move to the next block.
4. Adjust file position after each read operation.
5. Return the buffer content.

### **Writing to a File**

1. Add file descriptor index.
2. If file size = `0`, proceed to step 3; otherwise, step 4.
3. Assign the first free block to the control block, then proceed to step 5.
4. Set the first block address in the control block to `-1`.
5. Append data (determine block using file size and descriptor).
6. If additional blocks are required, allocate new ones.
7. Update the control block with the **new file size**.
8. Set the **write transaction flag**.
9. Count allocated blocks to determine the current file size.
10. If `new size ≤ current size`, proceed to step 12.
11. Locate the last block in FAT and set its value to the next free block.
12. Mark the **last block in FAT as ****`-1`**.
13. Update the new size in the **file descriptor**.
14. Clear the transaction flag.

**Recovery after interruption:** Restart from step 9.

---


