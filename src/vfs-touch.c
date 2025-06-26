// vfs-touch.c

#include "vfs.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
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
        
        // Verificar nombre válido
        if (!name_is_valid(filename)) {
            fprintf(stderr, "Nombre inválido: %s\n", filename);
            continue;
        }

        // Verificar si ya existe
        if (dir_lookup(image_path, filename) != 0) {
            fprintf(stderr, "El archivo '%s' ya existe\n", filename);
            continue;
        }

        // Crear archivo vacío con permisos por defecto
        int new_inode = create_empty_file_in_free_inode(image_path, DEFAULT_PERM);
        if (new_inode < 0) {
            fprintf(stderr, "Error al crear archivo '%s': %s\n", filename, strerror(errno));
            continue;
        }

        // Agregar entrada al directorio
        if (add_dir_entry(image_path, filename, new_inode) != 0) {
            fprintf(stderr, "Error al agregar entrada para '%s'\n", filename);
            free_inode(image_path, new_inode);
            continue;
        }

        printf("Archivo '%s' creado (inode %d)\n", filename, new_inode);
    }

    return EXIT_SUCCESS;
}
