// vfs-rm.c

#include "vfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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

        // Eliminar entrada del directorio
        if (remove_dir_entry(image_path, filename) != 0) {
            fprintf(stderr, "Error al eliminar entrada de directorio para '%s'\n", filename);
            continue;
        }

        // Liberar bloques de datos
        if (inode_trunc_data(image_path, &file_inode) != 0) {
            fprintf(stderr, "Error al liberar bloques de '%s'\n", filename);
            continue;
        }

        // Liberar inodo
        if (free_inode(image_path, inode_num) != 0) {
            fprintf(stderr, "Error al liberar inodo de '%s'\n", filename);
            continue;
        }

        printf("Archivo '%s' eliminado\n", filename);
    }

    return EXIT_SUCCESS;
}
