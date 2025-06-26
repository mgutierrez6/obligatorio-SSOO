// vfs-lsort.c

#include "vfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_ENTRIES 1024

typedef struct {
    uint32_t inode;
    char name[FILENAME_MAX_LEN];
    struct inode in;
} dir_entry_info;

int compare_entries(const void *a, const void *b) {
    const dir_entry_info *entry_a = (const dir_entry_info *)a;
    const dir_entry_info *entry_b = (const dir_entry_info *)b;
    return strcmp(entry_a->name, entry_b->name);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s imagen\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *image_path = argv[1];
    
    // Verificar imagen
    struct superblock sb_struct, *sb = &sb_struct;
    if (read_superblock(image_path, sb) != 0) {
        fprintf(stderr, "Error al leer superblock\n");
        return EXIT_FAILURE;
    }

    // Leer inodo del directorio raíz
    struct inode root_inode;
    if (read_inode(image_path, ROOTDIR_INODE, &root_inode) != 0) {
        fprintf(stderr, "Error al leer directorio raíz\n");
        return EXIT_FAILURE;
    }

    dir_entry_info entries[MAX_ENTRIES];
    int entry_count = 0;

    // Recorrer todos los bloques del directorio y recolectar entradas
    for (uint16_t i = 0; i < root_inode.blocks && entry_count < MAX_ENTRIES; i++) {
        int block_num = get_block_number_at(image_path, &root_inode, i);
        if (block_num <= 0) {
            fprintf(stderr, "Error al leer bloque %d del directorio\n", i);
            continue;
        }

        uint8_t data_buf[BLOCK_SIZE];
        if (read_block(image_path, block_num, data_buf) != 0) {
            fprintf(stderr, "Error al leer bloque %d\n", block_num);
            continue;
        }

        struct dir_entry *dir_entries = (struct dir_entry *)data_buf;
        
        for (uint32_t j = 0; j < DIR_ENTRIES_PER_BLOCK && entry_count < MAX_ENTRIES; j++) {
            if (dir_entries[j].inode != 0 && strcmp(dir_entries[j].name, ".") != 0 && 
                strcmp(dir_entries[j].name, "..") != 0) {
                
                if (read_inode(image_path, dir_entries[j].inode, &entries[entry_count].in) != 0) {
                    fprintf(stderr, "Error al leer inodo %u\n", dir_entries[j].inode);
                    continue;
                }
                
                entries[entry_count].inode = dir_entries[j].inode;
                strncpy(entries[entry_count].name, dir_entries[j].name, FILENAME_MAX_LEN);
                entry_count++;
            }
        }
    }

    // Ordenar entradas
    qsort(entries, entry_count, sizeof(dir_entry_info), compare_entries);

    // Mostrar entradas ordenadas
    printf("Total %u\n", root_inode.blocks);
    for (int i = 0; i < entry_count; i++) {
        print_inode(&entries[i].in, entries[i].inode, entries[i].name);
    }

    return EXIT_SUCCESS;
}
