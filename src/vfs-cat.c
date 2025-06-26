// vfs-cat.c

#include "vfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s imagen archivo1 [archivo2...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *image_path = argv[1];
    
    // Verificar imagen
    struct superblock sb_struct, *sb = &sb_struct;
    if (read_superblock(image_path, sb) != 0) {
        fprintf(stderr, "Error al leer superblock\n");
        return EXIT_FAILURE;
    }

    for (int i = 2; i < argc; i++) {
        const char *filename = argv[i];
        
        // Buscar el archivo en el directorio
        int inode_num = dir_lookup(image_path, filename);
        if (inode_num <= 0) {
            fprintf(stderr, "Archivo '%s' no encontrado\n", filename);
            continue;
        }

        // Leer inodo del archivo
        struct inode file_inode;
        if (read_inode(image_path, inode_num, &file_inode) != 0) {
            fprintf(stderr, "Error al leer inodo de '%s'\n", filename);
            continue;
        }

        // Verificar que sea un archivo regular
        if ((file_inode.mode & INODE_MODE_FILE) != INODE_MODE_FILE) {
            fprintf(stderr, "'%s' no es un archivo regular\n", filename);
            continue;
        }

        // Leer y mostrar el contenido
        uint8_t buffer[BUFFER_SIZE];
        size_t remaining = file_inode.size;
        size_t offset = 0;

        while (remaining > 0) {
            size_t to_read = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
            ssize_t nread = inode_read_data(image_path, inode_num, buffer, to_read, offset);
            
            if (nread <= 0) {
                fprintf(stderr, "Error al leer archivo '%s'\n", filename);
                break;
            }

            if (write(STDOUT_FILENO, buffer, nread) != nread) {
                fprintf(stderr, "Error al escribir a salida estándar\n");
                break;
            }

            offset += nread;
            remaining -= nread;
        }
    }

    return EXIT_SUCCESS;
}
