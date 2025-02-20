#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>

static int8_t NO_TRANSACTION_FLAG = -1;

/* BLOK KONTROLNY - USUWANIE */
static int8_t DELETE_TRANSACTION_FLAG = 1;
static int8_t TRANSACTION_FLAG_START = 0;
static int8_t DESC_ID_START = 1;
static int8_t CURR_BLOCK_START = 2;
static int8_t NEXT_BLOCK_START = 3;

/* TABLICA DESKRYPTOROW PLIKOW */
static int8_t DESC_TABLE_START = 5;
static int8_t FILE_NAME_START = 0;
static int8_t FIRST_BLOCK_ADDR_START = 8;
static int8_t FILE_SIZE_START = 9;
static int8_t DESC_TABLE_RECORD_SIZE = 11;

/* FAT */
static int16_t FAT_START = 205;
static int8_t UNUSED_BLOCK = -2;
static int8_t LAST_BLOCK = -1;

void delete_alg(std::fstream& file, int8_t first_block_addr, int8_t curr_block_addr, int8_t next_block_addr, bool interrupt)
{
    int8_t next_block;
    int8_t fat_curr;
    int8_t fat_next;
    do
    {
        file.seekg(FAT_START + curr_block_addr);
        file.read(reinterpret_cast<char*>(&next_block), sizeof(next_block));
        // Sprawdzam czy nie jest to ostatni blok
        if (next_block >= 0 && next_block_addr == curr_block_addr)
        {
            file.seekg(NEXT_BLOCK_START);
            file.write(reinterpret_cast<char*>(&next_block), sizeof(next_block));
            next_block_addr = next_block;
        }
        // Wstawiam -2
        file.seekg(FAT_START + curr_block_addr);
        file.write(reinterpret_cast<char*>(&UNUSED_BLOCK), sizeof(UNUSED_BLOCK));

        // Nagle przerwanie
        if (interrupt)
        {
            return;
        }

        // Przechodze dalej
        file.seekg(CURR_BLOCK_START);
        file.write(reinterpret_cast<char*>(&next_block_addr), sizeof(next_block_addr));

        curr_block_addr = next_block_addr;

        fat_curr = next_block;
        file.seekg(FAT_START + next_block_addr);
        file.read(reinterpret_cast<char*>(&fat_next), sizeof(fat_next));

        std::cout << int(fat_curr) << "\t" << int(fat_next) << std::endl;
    } while (fat_curr != UNUSED_BLOCK || fat_next != UNUSED_BLOCK);

    file.seekg(first_block_addr);
    file.write(reinterpret_cast<char*>(&UNUSED_BLOCK), sizeof(UNUSED_BLOCK));

    file.seekg(TRANSACTION_FLAG_START);
    file.write(reinterpret_cast<char*>(&NO_TRANSACTION_FLAG), sizeof(NO_TRANSACTION_FLAG));
}

void delete_recover(std::fstream& file)
{
    int8_t desc_id;
    file.seekg(DESC_ID_START);
    file.read(reinterpret_cast<char*>(&desc_id), sizeof(desc_id));
    int8_t desc_start = DESC_TABLE_START + DESC_TABLE_RECORD_SIZE * desc_id;
    int8_t first_block_addr = desc_start + FIRST_BLOCK_ADDR_START;
    int8_t curr_block_addr;
    int8_t next_block_addr;
    file.seekg(CURR_BLOCK_START);
    file.read(reinterpret_cast<char*>(&curr_block_addr), sizeof(curr_block_addr));
    file.seekg(NEXT_BLOCK_START);
    file.read(reinterpret_cast<char*>(&next_block_addr), sizeof(next_block_addr));
    delete_alg(file, first_block_addr, curr_block_addr, next_block_addr, false);
}

void recover(std::fstream& file)
{
    int8_t transaction_flag;
    file.seekg(TRANSACTION_FLAG_START);
    file.read(reinterpret_cast<char*>(&transaction_flag), sizeof(transaction_flag));

    if (transaction_flag == DELETE_TRANSACTION_FLAG)
    {
        delete_recover(file);
    }
}

void delete_file(std::fstream& file, int8_t desc_id, bool interrupt)
{
    int8_t desc_start = DESC_TABLE_START + DESC_TABLE_RECORD_SIZE * desc_id;
    int8_t first_block_addr = desc_start + FIRST_BLOCK_ADDR_START;
    int8_t first_block_addr_val;
    file.seekg(first_block_addr_val);
    file.read(reinterpret_cast<char*>(&first_block_addr_val), sizeof(first_block_addr_val));
    if(first_block_addr_val < 0)
    {
        file.seekg(first_block_addr);
        file.write(reinterpret_cast<char*>(&UNUSED_BLOCK), sizeof(UNUSED_BLOCK));
    }
    else
    {
        // Dodaje indeks deskryptora pliku do bloku kontrolnego
        file.seekg(DESC_ID_START);
        file.write(reinterpret_cast<char*>(&desc_id), sizeof(desc_id));

        // Dodaje adres obecnego i nastepnego bloku
        int8_t curr_block_addr;
        file.seekg(first_block_addr);
        file.read(reinterpret_cast<char*>(&curr_block_addr), sizeof(curr_block_addr));

        file.seekg(CURR_BLOCK_START);
        file.write(reinterpret_cast<char*>(&curr_block_addr), sizeof(curr_block_addr));
        file.seekg(NEXT_BLOCK_START);
        file.write(reinterpret_cast<char*>(&curr_block_addr), sizeof(curr_block_addr));

        // Dodaje flage usuwania
        file.seekg(TRANSACTION_FLAG_START);
        file.write(reinterpret_cast<char*>(&DELETE_TRANSACTION_FLAG), sizeof(DELETE_TRANSACTION_FLAG));

        delete_alg(file, first_block_addr, curr_block_addr, curr_block_addr, interrupt);
    }
}



int main() {
    int n;
    std::fstream disk("disk.bin", std::ios::in | std::ios::out | std::ios::binary);
    delete_file(disk, 0, true);
    disk.close();

    std::cin >> n;

    // std::fstream disk("disk.bin", std::ios::in | std::ios::out | std::ios::binary);
    disk.open("disk.bin", std::ios::in | std::ios::out | std::ios::binary);
    recover(disk);
    disk.close();
    return 0;
}