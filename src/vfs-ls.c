// vfs-ls.c

#include "vfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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

    printf("Total %u\n", root_inode.blocks);
    
    // Recorrer todos los bloques del directorio
    for (uint16_t i = 0; i < root_inode.blocks; i++) {
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

        struct dir_entry *entries = (struct dir_entry *)data_buf;
        
        // Mostrar cada entrada válida
        for (uint32_t j = 0; j < DIR_ENTRIES_PER_BLOCK; j++) {
            if (entries[j].inode != 0 && strcmp(entries[j].name, ".") != 0 && 
                strcmp(entries[j].name, "..") != 0) {
                
                struct inode file_inode;
                if (read_inode(image_path, entries[j].inode, &file_inode) != 0) {
                    fprintf(stderr, "Error al leer inodo %u\n", entries[j].inode);
                    continue;
                }
                
                print_inode(&file_inode, entries[j].inode, entries[j].name);
            }
        }
    }

    return EXIT_SUCCESS;
}
